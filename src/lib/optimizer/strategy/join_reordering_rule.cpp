#include "join_reordering_rule.hpp"

#include "join_ordering/dpsize.hpp"
#include "join_ordering/join_graph.hpp"
#include "optimizer/abstract_syntax_tree/abstract_ast_node.hpp"
#include "optimizer/abstract_syntax_tree/join_node.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

bool JoinReorderingRule::apply_to(const std::shared_ptr<AbstractASTNode>& node) {
  if (node->type() != ASTNodeType::Join) return _apply_to_children(node);

  auto root = node->parent();
  Assert(root, "Need a root node to attach result to");
  Assert(!root->right_child(), "Root node must only have one child");

  // Build join Graph
  auto join_graph = JoinGraph::build_join_graph(node);
  for (auto& join_node : join_graph->vertices()) {
    join_node->clear_parent();
  }

  // Invoke DPsize
  auto subroot = DPsize(join_graph).run();
  root->set_left_child(subroot);

  // Continue with vertices
  for (auto& vertex : join_graph->vertices()) {
    _apply_to_children(vertex);
  }

  return false;  // TODO(moritz)
}

//ResolvedColumn JoinReorderingRule::_resolve_column_id(const std::vector<std::shared_ptr<AbstractASTNode>>& nodes,
//                                                      ColumnID column_id) const {
//  // TODO(moritz) find something better than a linear search here
//  for (size_t node_idx = 0; node_idx < nodes.size(); ++node_idx) {
//    if (column_id < nodes[node_idx]->output_col_count()) {
//      return ResolvedColumn{column_id, node_idx};
//    }
//    column_id -= nodes[node_idx]->output_col_count();
//  }
//
//  Fail("Couldn't resolve column " + std::to_string(column_id));
//  return ResolvedColumn{INVALID_COLUMN_ID, 0};  // Return something to make compilers happy
//}
}