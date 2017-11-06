
#include "join_ordering_rule.hpp"

#include "join_ordering/greedy_join_ordering.hpp"
#include "optimizer/join_graph_builder.hpp"
#include "optimizer/abstract_syntax_tree/ast_utils.hpp"
#include "optimizer/abstract_syntax_tree/abstract_ast_node.hpp"
#include "optimizer/abstract_syntax_tree/join_node.hpp"
#include "optimizer/abstract_syntax_tree/mock_node.hpp"
#include "types.hpp"
#include "utils/assert.hpp"
#include "utils/assert.hpp"

namespace opossum {

std::string JoinOrderingRule::name() const {
  return "Join Reordering Rule";
}

bool JoinOrderingRule::apply_to(const std::shared_ptr<AbstractASTNode>& node) {
  /**
   * Identify parent and child_side. Later we will re-attach the newly ordered join plan to `parent` as the `child_side`
   * child.
   */
  auto parents = node->parents();
  auto child_sides = node->get_child_sides();

  /**
   * Without further safety measures (making sure a node doesn't receive a column mapping multiple times)
   * we can't apply the ordering to nodes with multiple parents
   */
  if (parents.size() > 1) {
    return _apply_to_children(node);
  }

  auto join_graph = JoinGraphBuilder().build_join_graph(node);

  // Nothing to order
  if (join_graph->vertices().size() <= 1) {
    return _apply_to_children(node);
  }

  auto parent = parents[0];
  auto child_side = child_sides[0];

  //
  const auto pre_ordering_column_origins = node->get_column_origins();

  // Remove all nodes above the vertices so we can freely reorder them.
  for (auto& vertex : join_graph->vertices()) {
    vertex.node->clear_parents();
  }

  // Invoke Ordering algorithm
  auto join_plan = GreedyJoinOrdering(join_graph).run();

  parent->set_child(child_side, join_plan);
  join_plan->dispatch_column_id_mapping(pre_ordering_column_origins);

  // Continue looking for Join Graphs to reorder in vertices
  for (auto& vertex : join_graph->vertices()) {
    _apply_to_children(vertex.node);
  }

  return false;  // TODO(moritz)
}
}