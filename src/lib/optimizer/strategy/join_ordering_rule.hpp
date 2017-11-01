#pragma once

#include <memory>
#include <vector>

#include "gtest/gtest_prod.h"

#include "abstract_rule.hpp"

namespace opossum {

class AbstractASTNode;

/**
 *
 */
class JoinOrderingRule : public AbstractRule {
 public:
  std::string name() const override;

  bool apply_to(const std::shared_ptr<AbstractASTNode>& node) override;
};

}  // namespace opossum
