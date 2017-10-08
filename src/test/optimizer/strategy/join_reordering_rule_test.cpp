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

  static void EXPECT_JOIN_EDGE(const std::vector<JoinEdge>& edges,
                               const std::vector<std::shared_ptr<AbstractASTNode>>& vertices,
                               const std::shared_ptr<AbstractASTNode>& node_a,
                               const std::shared_ptr<AbstractASTNode>& node_b, ColumnID column_id_a,
                               ColumnID column_id_b, ScanType scan_type) {
    for (const auto& edge : edges) {
      if (vertices.size() <= edge.node_indices.first) continue;
      if (vertices.size() <= edge.node_indices.second) continue;

      const auto& edge_node_a = vertices[edge.node_indices.first];
      const auto& edge_node_b = vertices[edge.node_indices.second];

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
   *      --Join--
   *    /         \
   *  table_a   table_b
   */
  auto join_node =
      std::make_shared<JoinNode>(JoinMode::Inner, std::make_pair(ColumnID{0}, ColumnID{2}), ScanType::OpEquals);
  auto table_a_node = std::make_shared<StoredTableNode>("table_a");
  auto table_b_node = std::make_shared<StoredTableNode>("table_b");

  join_node->set_left_child(table_a_node);
  join_node->set_right_child(table_b_node);

  std::vector<std::shared_ptr<AbstractASTNode>> vertices;
  std::vector<JoinEdge> edges;

  JoinReorderingRule::search_join_graph(join_node, vertices, edges);

  EXPECT_JOIN_VERTICES(vertices, {table_a_node, table_b_node});

  EXPECT_EQ(edges.size(), 1);
  EXPECT_JOIN_EDGE(edges, vertices, table_a_node, table_b_node, ColumnID{0}, ColumnID{2}, ScanType::OpEquals);
}

TEST_F(JoinReorderingRuleTest, SearchJoinGraphMedium) {
  /**
   * TODO(moritz) Split into multiple tests for search_join_graph invocations for different nodes
   */

  /**
   *                   Projection
   *                     |
   *     Join_a (table_a.a = table_c.b)
   *    /                              \
   *  table_a                   Join_b (table_b.a = table_c.c)
   *                           /                              \
   *                     table_b                              Predicate
   *                                                              |
   *                                                            table_c
   */
  auto projection_node = std::make_shared<ProjectionNode>(Expression::create_columns({ColumnID{0}}));
  auto join_a_node =
      std::make_shared<JoinNode>(JoinMode::Inner, std::make_pair(ColumnID{0}, ColumnID{5}), ScanType::OpGreaterThan);
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

  std::vector<std::shared_ptr<AbstractASTNode>> vertices;
  std::vector<JoinEdge> edges;

  // Searching from the root shouldn't yield anything except the root as it is not a join
  JoinReorderingRule::search_join_graph(projection_node, vertices, edges);
  EXPECT_JOIN_VERTICES(vertices, {projection_node});
  EXPECT_EQ(edges.size(), 0);

  // Searching from join_a should yield a non-empty join graph
  vertices.clear();
  edges.clear();
  JoinReorderingRule::search_join_graph(join_a_node, vertices, edges);

  EXPECT_JOIN_VERTICES(vertices, {table_a_node, table_b_node, predicate_node});

  EXPECT_EQ(edges.size(), 2);
  EXPECT_JOIN_EDGE(edges, vertices, table_a_node, table_c_node, ColumnID{0}, ColumnID{1}, ScanType::OpGreaterThan);
  EXPECT_JOIN_EDGE(edges, vertices, table_b_node, table_c_node, ColumnID{0}, ColumnID{2}, ScanType::OpEquals);
}
}