#include "dpsize.hpp"

#include <unordered_map>
#include <bitset>

#include "utils/assert.hpp"
#include "optimizer/abstract_syntax_tree/abstract_ast_node.hpp"
#include "optimizer/abstract_syntax_tree/join_node.hpp"
#include "optimizer/abstract_syntax_tree/predicate_node.hpp"
#include "optimizer/table_statistics.hpp"

namespace opossum {

DPsize::DPsize(const std::vector<std::shared_ptr<AbstractASTNode>> & tables,
       const std::vector<JoinEdge> & edges):
  m_tables(tables),
  m_edges(edges)
{

}

std::shared_ptr<AbstractASTNode> DPsize::run() {

  std::vector<JoinTableNode> table_nodes(m_tables.size());
  for (size_t table_idx = 0; table_idx < m_tables.size(); ++table_idx) {
    table_nodes[table_idx] = JoinTableNode{m_tables[table_idx], table_idx};
  }

  std::unordered_map<TableNodeSet, std::shared_ptr<JoinPlanNode>> best_plans;

  for (size_t table_idx = 0; table_idx < m_tables.size(); ++table_idx) {
    TableNodeSet node_set;
    node_set.set(table_idx);

    auto plan = std::make_shared<JoinPlanNode>();
    plan->table_idx = table_idx;
    plan->statistics = m_tables[table_idx]->get_statistics();

    best_plans[node_set] = plan;
  }

  for (size_t plan_size = 2; plan_size < m_tables.size(); ++plan_size) {
    for (size_t left_plan_size = 1; left_plan_size < plan_size; ++left_plan_size) {
      const auto right_plan_size = plan_size - left_plan_size;

      const auto left_sets = _generate_subsets_of_size(left_plan_size, m_tables.size());
      const auto right_sets = _generate_subsets_of_size(right_plan_size, m_tables.size());

      for (const auto & left_set : left_sets) {
        for (const auto & right_set : right_sets) {
          if ((left_set & right_set).any()) continue;

          const auto join_edges = _find_edges_between_sets(left_set, right_set);
          if (join_edges.empty()) continue;

          const auto iter1 = best_plans.find(left_set);
          Assert(iter1 != best_plans.end(), "");
          const auto best_plan_left = iter1->second;

          const auto iter2 = best_plans.find(right_set);
          Assert(iter2 != best_plans.end(), "");
          const auto best_plan_right = iter1->second;

          const auto plan = _join_plans(best_plan_left, best_plan_right, join_edges);

          const auto set = left_set | right_set;
          const auto iter3 = best_plans.find(set);

          if (iter3 == best_plans.end() || iter3->second->statistics->row_count() > plan->statistics->row_count()) {
            best_plans[set] = plan;
          }
        }
      }
    }
  }

  TableNodeSet complete_set;
  for (size_t bit_idx = 0; bit_idx < m_tables.size(); ++bit_idx) {
    complete_set.set(bit_idx);
  }

  auto iter = best_plans.find(complete_set);
  Assert(iter != best_plans.end(), "");
  auto best_plan = iter->second;

  return _build_join_tree(best_plan);
}

std::vector<TableNodeSet> DPsize::_generate_subsets_of_size(uint32_t set_size, uint32_t subset_size) const {
  std::vector<TableNodeSet> result;

  const auto max_plus_one = static_cast<uint32_t>(1 << (set_size - 1));

  for (uint32_t value = 0; value < max_plus_one; ++value) {
    TableNodeSet set(value);
    if (set.count() == subset_size) result.emplace_back(set);
  }

  return result;
}

std::vector<JoinEdge> DPsize::_find_edges_between_sets(const TableNodeSet &left, const TableNodeSet &right) const {
  // TODO(moritz) Can linear lookup be avoided?
  std::vector<JoinEdge> edges;

  for (const auto & edge : edges) {
    if (left.test(edge.node_indices.first) && right.test(edge.node_indices.second)) {
      edges.emplace_back(edge);
    } else if (right.test(edge.node_indices.first) && left.test(edge.node_indices.second)) {
      edges.emplace_back(edge);
    }
  }

  return edges;
}

std::shared_ptr<JoinPlanNode> DPsize::_join_plans(const std::shared_ptr<JoinPlanNode> & best_plan_left,
                                                  const std::shared_ptr<JoinPlanNode> & best_plan_right,
                                                  const std::vector<JoinEdge> & join_edges) const {
  auto min_row_count = std::numeric_limits<float>::max();
  auto min_row_count_edge_idx = size_t{0};

  auto best_plan_node = std::make_shared<JoinPlanNode>();
  best_plan_node->left_child = best_plan_left;
  best_plan_node->right_child = best_plan_right;

  for (size_t join_edge_idx = 0; join_edge_idx < join_edges.size(); ++join_edge_idx) {
    auto & edge = m_edges[join_edge_idx];

    auto statistics = best_plan_left->statistics->generate_predicated_join_statistics(
      best_plan_right->statistics, edge.predicate.mode, edge.predicate.column_ids, edge.predicate.scan_type
    );

    if (statistics->row_count() < min_row_count) {
      min_row_count = statistics->row_count();
      min_row_count_edge_idx = join_edge_idx;
      best_plan_node->statistics = statistics;
    }
  }

  best_plan_node->join_predicate = m_edges[min_row_count_edge_idx].predicate;

  for (size_t edge_idx = 0; edge_idx < m_edges.size(); ++edge_idx) {
    if (edge_idx == min_row_count_edge_idx) continue;
    best_plan_node->pending_predicates.emplace_back(m_edges[edge_idx].predicate);
  }

  return best_plan_node;
}

std::shared_ptr<AbstractASTNode> DPsize::_build_join_tree(const std::shared_ptr<JoinPlanNode> & plan_node) const {
  if (plan_node->table_idx) {
    return m_tables[*plan_node->table_idx];
  }

  auto join_node = std::make_shared<JoinNode>(plan_node->join_predicate.mode,
                                              plan_node->join_predicate.column_ids,
                                              plan_node->join_predicate.scan_type);

  auto root_node = std::static_pointer_cast<AbstractASTNode>(join_node);

  for (auto & predicate : plan_node->pending_predicates) {
    auto predicate_node = std::make_shared<PredicateNode>(predicate.column_ids.first, predicate.scan_type, predicate.column_ids.second);
    predicate_node->set_left_child(root_node);
    root_node = predicate_node;
  }

  join_node->set_left_child(_build_join_tree(plan_node->left_child));
  join_node->set_right_child(_build_join_tree(plan_node->right_child));

  return join_node;
}

}