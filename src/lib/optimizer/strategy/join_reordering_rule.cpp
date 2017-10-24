#include <optimizer/abstract_syntax_tree/mock_table_node.hpp>
#include "join_reordering_rule.hpp"

#include "join_ordering/dpsize.hpp"
#include "join_ordering/greedy_join_ordering.hpp"
#include "join_ordering/join_graph.hpp"
#include "optimizer/abstract_syntax_tree/ast_utils.hpp"
#include "optimizer/abstract_syntax_tree/abstract_ast_node.hpp"
#include "optimizer/abstract_syntax_tree/join_node.hpp"
#include "types.hpp"
#include "utils/assert.hpp"
#include "utils/assert.hpp"

namespace opossum {

std::string JoinReorderingRule::name() const {
  return "Join Reordering Rule";
}

bool JoinReorderingRule::apply_to(const std::shared_ptr<AbstractASTNode>& node) {
  if (node->type() != ASTNodeType::Join) return _apply_to_children(node);

  std::cout << "Applying Join Reordering" << std::endl;
  node->print();

  /**
   * Identify parent and child_side. Later we will re-attach the newly ordered join plan to `parent` as the `child_side`
   * child.
   */
  auto parent = node->parent();
  Assert(parent, "Need a parent node to attach result to");
  auto child_side = node->get_child_side();

  //
  const auto pre_ordering_column_origins = node->get_column_origins();

  // Build join Graph
  auto join_graph = JoinGraph::build_join_graph(node);

  // Remove all nodes above the vertices so we can freely reorder them.
  for (auto& edge_node : join_graph->vertices()) {
    edge_node->clear_parent();
  }

  // Invoke Ordering algorithm
  auto join_plan = GreedyJoinOrdering(join_graph).run();

  //
  const auto post_ordering_column_origins = join_plan->get_column_origins();
  const auto column_id_mapping = ast_generate_column_id_mapping(pre_ordering_column_origins, post_ordering_column_origins);

  // Re-attach the newly ordered join plan
  parent->reorder_columns(column_id_mapping);
  parent->set_child(child_side, join_plan);

  // Continue looking for Join Graphs to reorder in vertices
  for (auto& vertex : join_graph->vertices()) {
    _apply_to_children(vertex);
  }

  return false;  // TODO(moritz)
}
}