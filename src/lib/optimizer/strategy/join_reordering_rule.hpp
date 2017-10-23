#pragma once

#include <memory>
#include <vector>

#include "gtest/gtest_prod.h"

#include "abstract_rule.hpp"
#include "optimizer/strategy/join_ordering/join_graph.hpp"
#include "optimizer/strategy/join_ordering/join_ordering_types.hpp"

namespace opossum {

class AbstractASTNode;

/**
 *
 */
class JoinReorderingRule : public AbstractRule {
 public:
  std::string name() const override;

  bool apply_to(const std::shared_ptr<AbstractASTNode>& node) override;
};

}  // namespace opossum
