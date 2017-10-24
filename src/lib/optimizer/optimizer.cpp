#include "optimizer.hpp"

#include <memory>

#include "abstract_syntax_tree/ast_root_node.hpp"
#include "strategy/join_detection_rule.hpp"
#include "strategy/join_reordering_rule.hpp"
#include "strategy/predicate_reordering_rule.hpp"

namespace opossum {

const Optimizer& Optimizer::get() {
  static Optimizer optimizer;
  return optimizer;
}

Optimizer::Optimizer() {
  _rules.emplace_back(std::make_shared<PredicateReorderingRule>());
  _rules.emplace_back(std::make_shared<JoinDetectionRule>());
  _rules.emplace_back(std::make_shared<JoinReorderingRule>());
}

std::shared_ptr<AbstractASTNode> Optimizer::optimize(const std::shared_ptr<AbstractASTNode>& input) const {
  if (_verbose) {
    std::cout << "======== Optimizing =========" << std::endl;
    input->print();
    std::cout << "=============================" << std::endl;
  }

  /**
   * Add explicit root node, so the rules can freely change the tree below it without having to maintain a root node
   * to return to the Optimizer
   */
  const auto root_node = std::make_shared<ASTRootNode>();
  root_node->set_left_child(input);

  /**
   * Apply all optimization over and over until all of them stopped changing the AST or the max number of
   * iterations is reached
   */
  for (uint32_t iter_index = 0; iter_index < _max_num_iterations; ++iter_index) {
    if (_verbose) {
      std::cout << "====== Iteration " << iter_index << " ========" << std::endl;
      input->print();
      std::cout << "=============================" << std::endl;
    }
    auto ast_changed = false;

    for (const auto& rule : _rules) {
      ast_changed |= rule->apply_to(root_node);
      if (_verbose) {
        std::cout << "====== After '" << rule->name() << "' ========" << std::endl;
        root_node->left_child()->print();
        std::cout << "=============================" << std::endl;
      }
    }

    if (!ast_changed) break;
  }

  // Remove ASTRootNode
  const auto optimized_node = root_node->left_child();
  optimized_node->clear_parent();

  return optimized_node;
}

}  // namespace opossum
