#include "join_reordering_base_test.hpp"

#include "optimizer/strategy/join_ordering/greedy_join_ordering.hpp"

namespace opossum {

class GreedyJoinOrderingTest : public JoinReorderingBaseTest {
 public:

};

TEST_F(GreedyJoinOrderingTest, SimpleGraph) {
  /**
   * The chain equi-JoinGraph {C, D, E} should result in this JoinPlan:
   *
   *         ___Join (D.a == E.a)___
   *        /                       \
   *   ___Join (C.a == D.a)___       E
   *  /                       \
   * C                        D
   */

  auto plan = GreedyJoinOrdering(_join_graph_cde_chain).run();

  ASSERT_INNER_JOIN_NODE(plan, ScanType::OpEquals, ColumnID{1}, ColumnID{0});
  ASSERT_INNER_JOIN_NODE(plan->left_child(), ScanType::OpEquals, ColumnID{0}, ColumnID{0});
  ASSERT_EQ(plan->right_child(), _table_node_e);
  ASSERT_EQ(plan->left_child()->left_child(), _table_node_c);
  ASSERT_EQ(plan->left_child()->right_child(), _table_node_d);
}

}