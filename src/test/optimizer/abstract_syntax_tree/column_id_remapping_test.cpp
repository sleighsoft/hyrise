#include "gtest/gtest.h"

#include <memory>

#include "optimizer/abstract_syntax_tree/ast_utils.hpp"
#include "optimizer/abstract_syntax_tree/join_node.hpp"
#include "optimizer/abstract_syntax_tree/mock_table_node.hpp"
#include "optimizer/abstract_syntax_tree/predicate_node.hpp"

namespace opossum {

class ColumnIDRemappingTest : public ::testing::Test {
 public:
  void SetUp() override {
    _table_a = std::make_shared<MockTableNode>("a", 2);
    _table_b = std::make_shared<MockTableNode>("b", 4);
    _table_c = std::make_shared<MockTableNode>("c", 3);


  }

 protected:
  std::shared_ptr<MockTableNode> _table_a;
  std::shared_ptr<MockTableNode> _table_b;
  std::shared_ptr<MockTableNode> _table_c;
};

TEST_F(ColumnIDRemappingTest, Basic) {
  /**
   *    Predicate (a.1 == b.1)
   *            |
   *     ([0..1]=A, [2..5]=B)
   *            |
   *        CrossJoin
   *       /         \
   *      A           B
   *
   * reorder to:
   *
   *    Predicate (a.1 == b.1)
   *            |
   *      ([0..3]=B, [4..5]=A)
   *            |
   *        CrossJoin
   *       /         \
   *      B           A
   */

  const auto join_plan_a = std::make_shared<JoinNode>(JoinMode::Cross);
  const auto predicate = std::make_shared<PredicateNode>(ColumnID{1}, ScanType::OpEquals, ColumnID{3});

  join_plan_a->set_children(_table_a, _table_b);
  predicate->set_left_child(join_plan_a);

  const auto column_origins = join_plan_a->get_column_origins();

  const auto join_plan_b = std::make_shared<JoinNode>(JoinMode::Cross);
  join_plan_b->set_children(_table_b, _table_a);

  predicate->set_left_child(join_plan_b);
  predicate->reorder_columns(column_origins);

  EXPECT_EQ(predicate->column_id(), ColumnID{5});
  EXPECT_EQ(boost::get<ColumnID>(predicate->value()), ColumnID{1});
}

}  // namespace opossum