#include "join_reordering_rule.hpp"
#include <optimizer/abstract_syntax_tree/join_node.hpp>

#include "join_ordering/dpsize.hpp"
#include "optimizer/abstract_syntax_tree/abstract_ast_node.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

bool JoinReorderingRule::apply_to(const std::shared_ptr<AbstractASTNode>& node) {
  if (node->type() != ASTNodeType::Join) return _apply_to_children(node);

  auto root = node->parent();
  Assert(root, "Need a root node to attach result to");
  Assert(!root->right_child(), "Root node must only have one child");

  // Build join Graph
  std::vector<std::shared_ptr<AbstractASTNode>> join_vertices;
  std::vector<JoinEdge> join_edges;
  /*const auto reordered_descendants = */ search_join_graph(node, join_vertices, join_edges, ColumnID{0});
  for (auto& join_node : join_vertices) {
    join_node->clear_parent();
  }

  // Resolve the edges node_idx and the local column ids
  for (auto& join_edge : join_edges) {
    const auto left_resolved_column_id = _resolve_column_id(join_vertices, join_edge.predicate.column_ids.first);
    const auto right_resolved_column_id = _resolve_column_id(join_vertices, join_edge.predicate.column_ids.second);
    join_edge.predicate.column_ids.first = left_resolved_column_id.column_id;
    join_edge.node_indices.first = left_resolved_column_id.node_idx;
    join_edge.predicate.column_ids.second = right_resolved_column_id.column_id;
    join_edge.node_indices.second = right_resolved_column_id.node_idx;
  }

  // Invoke DPsize
  auto subroot = DPsize(join_vertices, join_edges).run();
  root->set_left_child(subroot);

  // Continue with vertices
  for (auto& vertex : join_vertices) {
    _apply_to_children(vertex);
  }

  return false;  // TODO(moritz)
}

JoinGraph JoinReorderingRule::search_join_graph(const std::shared_ptr<AbstractASTNode>& node) {
  JoinGraph::Vertices vertices;
  JoinGraph::Edges edges;

  search_join_graph(node, vertices, edges);

  return JoinGraph(std::move(vertices), std::move(edges));
}


void JoinReorderingRule::search_join_graph(const std::shared_ptr<AbstractASTNode>& node,
                              std::vector<std::shared_ptr<AbstractASTNode>>& o_vertices,
                              std::vector<JoinEdge>& o_edges, ColumnID column_id_offset) {
  // To make it possible to call search_join_graph() on both children without having to check whether they are nullptr.
  if (!node) return;

  // Everything that is not a Join becomes a vertex
  if (node->type() != ASTNodeType::Join) {
    o_vertices.emplace_back(node);
    return;
  }

  const auto join_node = std::static_pointer_cast<JoinNode>(node);

  // Every non-inner join becomes a vertex for now
  if (join_node->join_mode() != JoinMode::Inner) {
    o_vertices.emplace_back(node);
    return;
  }

  Assert(join_node->scan_type(), "Need scan type for now, since only inner joins are supported");
  Assert(join_node->join_column_ids(), "Need join columns for now, since only inner joins are supported");

  /**
   * Process children on the left side
   */
  const auto left_vertex_offset = o_vertices.size();
  search_join_graph(node->left_child(), o_vertices, o_edges, column_id_offset);

  /**
   * Process children on the right side
   */
  const auto right_column_offset =
      ColumnID{static_cast<ColumnID::base_type>(column_id_offset + node->left_child()->output_col_count())};
  const auto right_vertex_offset = o_vertices.size();
  search_join_graph(node->right_child(), o_vertices, o_edges, right_column_offset);

  /**
   * Build the JoinEdge corresponding to this node
   */
  JoinEdge edge;
  edge.predicate.scan_type = *join_node->scan_type();
  edge.predicate.mode = join_node->join_mode();

  auto left_column_id = join_node->join_column_ids()->first;
  auto right_column_id = join_node->join_column_ids()->second;

  // Find vertex indices
  const auto find_column = [&o_vertices] (auto column_id, const auto vertex_range_begin, const auto vertex_range_end) {
    for (JoinVertexId vertex_idx = vertex_range_begin; vertex_idx < vertex_range_end; ++vertex_idx) {
      const auto &vertex = o_vertices[vertex_idx];
      if (column_id < vertex->output_col_count()) {
        return std::make_pair(vertex_idx, column_id);
      }
      column_id -= vertex->output_col_count();
    }
    Fail("Couldn't find column_id in vertex range.");
    return std::make_pair(INVALID_JOIN_VERTEX_ID, INVALID_COLUMN_ID);
  };

  const auto find_left_result = find_column(left_column_id, left_vertex_offset, right_vertex_offset);
  const auto find_right_result = find_column(right_column_id, right_vertex_offset, o_vertices.size());

  edge.node_indices.first = find_left_result.first;
  edge.predicate.column_ids.first = find_left_result.second;
  edge.node_indices.second = find_right_result.first;
  edge.predicate.column_ids.second = find_right_result.second;

  o_edges.emplace_back(edge);
}

ResolvedColumn JoinReorderingRule::_resolve_column_id(const std::vector<std::shared_ptr<AbstractASTNode>>& nodes,
                                                      ColumnID column_id) const {
  // TODO(moritz) find something better than a linear search here
  for (size_t node_idx = 0; node_idx < nodes.size(); ++node_idx) {
    if (column_id < nodes[node_idx]->output_col_count()) {
      return ResolvedColumn{column_id, node_idx};
    }
    column_id -= nodes[node_idx]->output_col_count();
  }

  Fail("Couldn't resolve column " + std::to_string(column_id));
  return ResolvedColumn{INVALID_COLUMN_ID, 0};  // Return something to make compilers happy
}
}