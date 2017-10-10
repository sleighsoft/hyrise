#include "optimizer/strategy/join_reordering_base_test.hpp"

#include "optimizer/strategy/join_ordering/dpsize.hpp"
#include "planviz/ast_visualizer.hpp"

namespace opossum {

class DPsizeTest: public JoinReorderingBaseTest {
 public:
  void SetUp() override {

  }

  void TearDown() override {

  }
};

TEST_F(DPsizeTest, Basics) {
  auto root = DPsize(_join_graph_a).run();

  ASTVisualizer::visualize({root}, "dpsize.dot", "dpsize.png");
}

}