#include "base_test.hpp"

#include <algorithm>

#include "optimizer/abstract_syntax_tree/join_node.hpp"
#include "optimizer/abstract_syntax_tree/predicate_node.hpp"
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
    StorageManager::get().add_table("table_c", load_table("src/test/tables/int_int_int.tbl", 0));
  }

  void TearDown() override {}

  static void EXPECT_JOIN_EDGE(const JoinGraph &join_graph,
                               const std::shared_ptr<AbstractASTNode>& node_a,
                               const std::shared_ptr<AbstractASTNode>& node_b, ColumnID column_id_a,
                               ColumnID column_id_b, ScanType scan_type) {
    for (const auto& edge : join_graph.edges()) {
      if (join_graph.vertices().size() <= edge.node_indices.first) continue;
      if (join_graph.vertices().size() <= edge.node_indices.second) continue;

      const auto& edge_node_a = join_graph.vertices()[edge.node_indices.first];
      const auto& edge_node_b = join_graph.vertices()[edge.node_indices.second];

      if (edge_node_a == node_a) {
        if (edge_node_b == node_b && edge.predicate.column_ids == std::make_pair(column_id_a, column_id_b)) {
          return;  // we found a matching edge
        }
      } else {
        if (edge_node_a == node_b && edge_node_b == node_a &&
            edge.predicate.column_ids == std::make_pair(column_id_b, column_id_a)) {
          return;
        }
      }
    }

    FAIL();  // Couldn't find a matching edge
  }

  static void EXPECT_JOIN_VERTICES(const std::vector<std::shared_ptr<AbstractASTNode>>& vertices_a,
                                   const std::vector<std::shared_ptr<AbstractASTNode>>& vertices_b) {
    EXPECT_TRUE(std::equal(vertices_a.begin(), vertices_a.end(), vertices_b.begin(), vertices_b.end()));
  }
};

TEST_F(JoinReorderingRuleTest, SearchJoinGraphSimple) {
  /**
   *      --Join (table_a.a = table_b.b) --
   *    /                                  \
   *  table_a                               table_b
   */
  auto join_node =
      std::make_shared<JoinNode>(JoinMode::Inner, std::make_pair(ColumnID{0}, ColumnID{1}), ScanType::OpEquals);
  auto table_a_node = std::make_shared<StoredTableNode>("table_a");
  auto table_b_node = std::make_shared<StoredTableNode>("table_b");

  join_node->set_left_child(table_a_node);
  join_node->set_right_child(table_b_node);

  const auto join_graph = JoinReorderingRule::search_join_graph(join_node);

  join_graph.print();

  EXPECT_JOIN_VERTICES(join_graph.vertices(), {table_a_node, table_b_node});

  EXPECT_EQ(join_graph.edges().size(), 1);
  EXPECT_JOIN_EDGE(join_graph, table_a_node, table_b_node, ColumnID{0}, ColumnID{1}, ScanType::OpEquals);
}

TEST_F(JoinReorderingRuleTest, SearchJoinGraphMedium) {
  /**
   * TODO(moritz) Split into multiple tests for search_join_graph invocations for different nodes
   */

  /**
   *                   Projection
   *                     |
   *     Join_a (table_a.a > table_c.b)
   *    /                              \
   *  table_a                   Join_b (table_b.a = table_c.c)
   *                           /                              \
   *                     table_b                              Predicate
   *                                                              |
   *                                                            table_c
   */
  auto projection_node = std::make_shared<ProjectionNode>(Expression::create_columns({ColumnID{0}}));
  auto join_a_node =
      std::make_shared<JoinNode>(JoinMode::Inner, std::make_pair(ColumnID{0}, ColumnID{3}), ScanType::OpGreaterThan);
  auto join_b_node =
      std::make_shared<JoinNode>(JoinMode::Inner, std::make_pair(ColumnID{0}, ColumnID{2}), ScanType::OpEquals);
  auto table_a_node = std::make_shared<StoredTableNode>("table_a");
  auto table_b_node = std::make_shared<StoredTableNode>("table_b");
  auto table_c_node = std::make_shared<StoredTableNode>("table_c");
  auto predicate_node = std::make_shared<PredicateNode>(ColumnID{0}, ScanType::OpGreaterThan, 4);

  predicate_node->set_left_child(table_c_node);
  join_b_node->set_left_child(table_b_node);
  join_b_node->set_right_child(predicate_node);
  join_a_node->set_left_child(table_a_node);
  join_a_node->set_right_child(join_b_node);
  projection_node->set_left_child(join_a_node);

  // Searching from the root shouldn't yield anything except the root as it is not a join
  const auto join_graph_a = JoinReorderingRule::search_join_graph(projection_node);
  EXPECT_JOIN_VERTICES(join_graph_a.vertices(), {projection_node});
  EXPECT_EQ(join_graph_a.edges().size(), 0);

  join_graph_a.print();

  // Searching from join_a should yield a non-empty join graph
  const auto join_graph_b = JoinReorderingRule::search_join_graph(join_a_node);

  join_graph_b.print();

  EXPECT_JOIN_VERTICES(join_graph_b.vertices(), {table_a_node, table_b_node, predicate_node});

  EXPECT_EQ(join_graph_b.edges().size(), 2);
  EXPECT_JOIN_EDGE(join_graph_b, table_a_node, predicate_node, ColumnID{0}, ColumnID{1}, ScanType::OpGreaterThan);
  EXPECT_JOIN_EDGE(join_graph_b, table_b_node, predicate_node, ColumnID{0}, ColumnID{2}, ScanType::OpEquals);
}
}