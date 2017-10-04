#include "base_test.hpp"

#include "optimizer/abstract_syntax_tree/predicate_node.hpp"
#include "optimizer/abstract_syntax_tree/join_node.hpp"
#include "optimizer/abstract_syntax_tree/projection_node.hpp"
#include "optimizer/abstract_syntax_tree/stored_table_node.hpp"
#include "optimizer/strategy/join_reordering_rule.hpp"
#include "storage/storage_manager.hpp"

namespace opossum {

class JoinReorderingRuleTest : public BaseTest {
 protected:
  void SetUp() override {
    StorageManager::get().add_table("table_a", load_table("src/test/tables/int.tbl", 0));
    StorageManager::get().add_table("table_b", load_table("src/test/tables/int_int.tbl", 0));

  }

  void TearDown() override {

  }

  static void EXPECT_JOIN_EDGE(const JoinEdge & edge,
                               const std::vector<std::shared_ptr<AbstractASTNode>> & vertices,
                               const std::shared_ptr<AbstractASTNode> & node_a,
                               const std::shared_ptr<AbstractASTNode> & node_b,
                               const std::pair<ColumnID, ColumnID> & column_ids,
                               ScanType scan_type) {
    ASSERT_GT(vertices.size(), edge.node_indices.first);
    ASSERT_GT(vertices.size(), edge.node_indices.second);

    const auto & edge_node_a = vertices[edge.node_indices.first];
    const auto & edge_node_b = vertices[edge.node_indices.second];

    if (edge_node_a == node_a) {
      EXPECT_EQ(edge_node_b, node_b);
      EXPECT_EQ(edge.predicate.column_ids, column_ids);
    } else {
      EXPECT_EQ(edge_node_a, node_b);
      EXPECT_EQ(edge_node_b, node_a);
      EXPECT_EQ(edge.predicate.column_ids, std::make_pair(column_ids.second, column_ids.first));
    }
  }
};

TEST_F(JoinReorderingRuleTest, SearchJoinGraphSimple) {
  /**
   *      --Join--
   *    /         \
   *  table_a   table_b
   */
  auto join_node = std::make_shared<JoinNode>(JoinMode::Inner, std::make_pair(ColumnID{0}, ColumnID{2}), ScanType::OpEquals);
  auto table_a_node = std::make_shared<StoredTableNode>("table_a");
  auto table_b_node = std::make_shared<StoredTableNode>("table_b");

  join_node->set_left_child(table_a_node);
  join_node->set_right_child(table_b_node);

  std::vector<std::shared_ptr<AbstractASTNode>> vertices;
  std::vector<JoinEdge> edges;

  JoinReorderingRule::search_join_graph(join_node, vertices, edges);

  ASSERT_EQ(vertices.size(), 2);
  ASSERT_EQ(vertices[0]->type(), ASTNodeType::StoredTable);
  ASSERT_EQ(vertices[1]->type(), ASTNodeType::StoredTable);

  auto names = std::vector<std::string>{
    std::dynamic_pointer_cast<StoredTableNode>(vertices[0])->table_name(),
    std::dynamic_pointer_cast<StoredTableNode>(vertices[1])->table_name()
  };

  std::sort(names.begin(), names.end());

  EXPECT_EQ(names[0], "table_a");
  EXPECT_EQ(names[1], "table_b");

  ASSERT_EQ(edges.size(), 1);

  EXPECT_JOIN_EDGE(edges[0], vertices, table_a_node, table_b_node,
                   std::make_pair(ColumnID{0}, ColumnID{2}), ScanType::OpEquals);
}

TEST_F(JoinReorderingRuleTest, SearchJoinGraphMedium) {
//  /**
//   *     Projection
//   *          |
//   *     --Join(a)--
//   *    /            \
//   *  table_a        Join(b)
//   *                /      \
//   *         table_b       Predicate
//   *                           |
//   *                        table_c
//   *
//   */
//  auto projection = std::make_shared<ProjectionNode>({Expression::create_column(ColumnID{0})});
//  projection->set_left_child(std::make_shared<StoredTableNode>("table_a"));
//
//  auto join_a = std::make_shared<JoinNode>(JoinMode::Inner, std::make_pair(ColumnID{0}, ColumnID{2}), ScanType::OpEquals);
//
//  auto plan_a = std::make_shared<JoinNode>(JoinMode::Inner, std::make_pair(ColumnID{0}, ColumnID{2}), ScanType::OpEquals);
//  plan_a->set_left_child(std::make_shared<StoredTableNode>("table_a"));
//  plan_a->set_right_child(std::make_shared<StoredTableNode>("table_b"));
//
//  std::vector<std::shared_ptr<AbstractASTNode>> vertices;
//  std::vector<JoinEdge> edges;
//
//  JoinReorderingRule::_search_join_graph(plan_a, vertices, edges, ColumnID{0});
//
//  ASSERT_EQ(vertices.size(), 2);
//  ASSERT_EQ(vertices[0]->type(), ASTNodeType::StoredTable);
//  ASSERT_EQ(vertices[1]->type(), ASTNodeType::StoredTable);
//
//  auto names = std::vector<std::string>{
//    std::dynamic_pointer_cast<StoredTableNode>(vertices[0])->table_name(),
//    std::dynamic_pointer_cast<StoredTableNode>(vertices[1])->table_name()
//  };
//
//  std::sort(names.begin(), names.end());
//
//  EXPECT_EQ(names[0], "table_a");
//  EXPECT_EQ(names[1], "table_b");
}

}