#include "gtest/gtest.h"

#include "optimizer/abstract_syntax_tree/ast_utils.hpp"
#include "optimizer/abstract_syntax_tree/join_node.hpp"
#include "optimizer/abstract_syntax_tree/mock_table_node.hpp"
#include "optimizer/abstract_syntax_tree/predicate_node.hpp"

namespace opossum {

class ASTUtilsTest : public ::testing::Test {
 public:
  void SetUp() override {
    /**
     * Build this AST:
     *
     *    JoinB (C[6] >= A[2])
     *   /                   \
     *  C                    PredicateB (A[1] > B[1])
     *                                  |
     *                        JoinA (B[1] >= A[2])
     *                       /                    \
     *                      B                      PredicateA (A[0] == 5)
     *                                                      |
     *                                                      A
     */

    _table_a = std::make_shared<MockTableNode>("a", 3);
    _table_b = std::make_shared<MockTableNode>("b", 2);
    _table_c = std::make_shared<MockTableNode>("c", 8);

    _predicate_a = std::make_shared<PredicateNode>(ColumnID{0}, ScanType::OpEquals, 5);
    _predicate_a->set_left_child(_table_a);

    _join_a = std::make_shared<JoinNode>(JoinMode::Inner, std::make_pair(ColumnID{1}, ColumnID{2}),
                                         ScanType::OpGreaterThanEquals);
    _join_a->set_children(_table_b, _predicate_a);

    _predicate_b = std::make_shared<PredicateNode>(ColumnID{3}, ScanType::OpGreaterThan, ColumnID{1});
    _predicate_b->set_left_child(_join_a);

    _join_b = std::make_shared<JoinNode>(JoinMode::Inner, std::make_pair(ColumnID{6}, ColumnID{4}),
                                         ScanType::OpGreaterThanEquals);
    _join_b->set_children(_table_c, _predicate_b);
  }

 protected:
  std::shared_ptr<MockTableNode> _table_a;
  std::shared_ptr<MockTableNode> _table_b;
  std::shared_ptr<MockTableNode> _table_c;
  std::shared_ptr<PredicateNode> _predicate_a;
  std::shared_ptr<PredicateNode> _predicate_b;
  std::shared_ptr<JoinNode> _join_a;
  std::shared_ptr<JoinNode> _join_b;
};

TEST_F(ASTUtilsTest, GetFirstColumnIDOfDescendant) {
  EXPECT_EQ(ast_get_first_column_id_of_descendant(_table_a, _table_a), ColumnID{0});
  EXPECT_EQ(ast_get_first_column_id_of_descendant(_table_a, _table_c), INVALID_COLUMN_ID);

  EXPECT_EQ(ast_get_first_column_id_of_descendant(_predicate_a, _table_a), ColumnID{0});
  EXPECT_EQ(ast_get_first_column_id_of_descendant(_predicate_a, _table_c), INVALID_COLUMN_ID);

  EXPECT_EQ(ast_get_first_column_id_of_descendant(_join_a, _table_a), ColumnID{2});
  EXPECT_EQ(ast_get_first_column_id_of_descendant(_join_a, _table_b), ColumnID{0});
  EXPECT_EQ(ast_get_first_column_id_of_descendant(_join_a, _table_c), INVALID_COLUMN_ID);

  EXPECT_EQ(ast_get_first_column_id_of_descendant(_join_b, _join_b), ColumnID{0});
  EXPECT_EQ(ast_get_first_column_id_of_descendant(_join_b, _table_c), ColumnID{0});
  EXPECT_EQ(ast_get_first_column_id_of_descendant(_join_b, _table_b), ColumnID{8});
  EXPECT_EQ(ast_get_first_column_id_of_descendant(_join_b, _table_a), ColumnID{10});
  EXPECT_EQ(ast_get_first_column_id_of_descendant(_join_b, _predicate_a), ColumnID{10});
  EXPECT_EQ(ast_get_first_column_id_of_descendant(_join_b, _join_a), ColumnID{8});
}

TEST_F(ASTUtilsTest, ContainsJoinEdge) {
  EXPECT_TRUE(
      ast_contains_join_edge(_join_a, _table_b, _table_a, ColumnID{1}, ColumnID{2}, ScanType::OpGreaterThanEquals));
  EXPECT_TRUE(
      ast_contains_join_edge(_join_a, _table_a, _table_b, ColumnID{2}, ColumnID{1}, ScanType::OpLessThanEquals));
  EXPECT_TRUE(
      ast_contains_join_edge(_predicate_b, _table_a, _table_b, ColumnID{2}, ColumnID{1}, ScanType::OpLessThanEquals));
  EXPECT_TRUE(
      ast_contains_join_edge(_join_b, _table_a, _table_b, ColumnID{2}, ColumnID{1}, ScanType::OpLessThanEquals));
  EXPECT_FALSE(
      ast_contains_join_edge(_join_a, _table_a, _table_b, ColumnID{2}, ColumnID{1}, ScanType::OpGreaterThanEquals));
  EXPECT_FALSE(
      ast_contains_join_edge(_join_a, _table_a, _table_b, ColumnID{2}, ColumnID{2}, ScanType::OpLessThanEquals));

  EXPECT_TRUE(
      ast_contains_join_edge(_join_b, _table_a, _table_c, ColumnID{2}, ColumnID{6}, ScanType::OpLessThanEquals));
  EXPECT_TRUE(
      ast_contains_join_edge(_join_b, _table_c, _table_a, ColumnID{6}, ColumnID{2}, ScanType::OpGreaterThanEquals));

  EXPECT_FALSE(ast_contains_join_edge(_join_b, _table_c, _table_a, ColumnID{9}, ColumnID{5}, ScanType::OpEquals));
}
}