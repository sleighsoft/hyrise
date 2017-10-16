#pragma once

#include <memory>
#include <vector>

#include "gtest/gtest_prod.h"

#include "abstract_rule.hpp"
#include "optimizer/strategy/join_ordering/join_graph.hpp"
#include "optimizer/strategy/join_ordering/join_ordering_types.hpp"

namespace opossum {

class AbstractASTNode;

struct ResolvedColumn {
  ColumnID column_id = INVALID_COLUMN_ID;
  size_t node_idx = 0;
};

/**
 *
 */
class JoinReorderingRule : public AbstractRule {
 public:
  bool apply_to(const std::shared_ptr<AbstractASTNode>& node) override;

 private:
  ResolvedColumn _resolve_column_id(const std::vector<std::shared_ptr<AbstractASTNode>>& nodes,
                                    ColumnID column_id) const;
};

}  // namespace opossum
