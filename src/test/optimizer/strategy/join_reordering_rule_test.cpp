#include "base_test.hpp"

#include "optimizer/abstract_syntax_tree/predicate_node.hpp"
#include "optimizer/abstract_syntax_tree/join_node.hpp"
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
};

TEST_F(JoinReorderingRuleTest, SearchJoinGraph) {
  /**
   *      --Join--
   *    /         \
   *  table_a   table_B
   */
  auto plan_a = std::make_shared<JoinNode>(JoinMode::Inner, std::make_pair(ColumnID{0}, ColumnID{2}), ScanType::OpEquals);
  plan_a->set_left_child(std::make_shared<StoredTableNode>("table_a"));
  plan_a->set_right_child(std::make_shared<StoredTableNode>("table_b"));

  std::vector<std::shared_ptr<AbstractASTNode>> vertices;
  std::vector<JoinEdge> edges;

  JoinReorderingRule::_search_join_graph(plan_a, vertices, edges, ColumnID{0});


}

}