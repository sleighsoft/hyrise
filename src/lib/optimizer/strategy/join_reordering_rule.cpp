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

bool JoinReorderingRule::search_join_graph(const std::shared_ptr<AbstractASTNode>& node,
                                           std::vector<std::shared_ptr<AbstractASTNode>>& o_vertices,
                                           std::vector<JoinEdge>& o_edges, ColumnID column_id_offset) {
  if (!node) return false;
  if (node->type() != ASTNodeType::Join) {
    o_vertices.emplace_back(node);
    return true;
  }

  const auto join_node = std::static_pointer_cast<JoinNode>(node);
  Assert(join_node->scan_type(), "Need scan type for now");
  Assert(join_node->join_column_ids(), "Need join columns for now");

  /**
   * Not using `_search_join_nodes(node->left_child()) ||  _search_join_nodes(node->right_child())` because of
   * short-circuited evaluation. If the left side of the expression is true, the right side would not be called.
   */
  const auto reordered_descendants_left = search_join_graph(node->left_child(), o_vertices, o_edges, column_id_offset);

  const auto right_column_offset =
      ColumnID{static_cast<ColumnID::base_type>(column_id_offset + node->left_child()->output_col_count())};
  const auto reordered_descendants_right =
      search_join_graph(node->right_child(), o_vertices, o_edges, right_column_offset);

  auto left_column_id = join_node->join_column_ids()->first;
  auto right_column_id = join_node->join_column_ids()->second;

  JoinEdge edge;
  edge.predicate.scan_type = *join_node->scan_type();
  edge.predicate.mode = join_node->join_mode();
  edge.predicate.column_ids = std::make_pair(left_column_id + column_id_offset, right_column_id + column_id_offset);

  // Find vertex indices
  left_column_id += column_id_offset;
  right_column_id += column_id_offset;
  for (JoinVertexId vertex_id = 0; vertex_id < o_vertices.size(); ++vertex_id) {
    auto& vertex_node = o_vertices[vertex_id];

    if (edge.node_indices.first == INVALID_JOIN_VERTEX_ID && left_column_id < vertex_node->output_col_count()) {
      edge.node_indices.first = vertex_id;
    }
    left_column_id -= vertex_node->output_col_count();  // This might underflow, but only after we found the vertex

    if (edge.node_indices.second == INVALID_JOIN_VERTEX_ID && right_column_id < vertex_node->output_col_count()) {
      edge.node_indices.second = vertex_id;
    }
    right_column_id -= vertex_node->output_col_count();  // This might underflow, but only after we found the vertex
  }

  Assert(edge.node_indices.first != INVALID_JOIN_VERTEX_ID, "Couldn't find vertex corresponding to column id");
  Assert(edge.node_indices.second != INVALID_JOIN_VERTEX_ID, "Couldn't find vertex corresponding to column id");

  o_edges.emplace_back(edge);

  return reordered_descendants_left || reordered_descendants_right;
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