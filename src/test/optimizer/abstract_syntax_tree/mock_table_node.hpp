#pragma once

#include "optimizer/abstract_syntax_tree/abstract_ast_node.hpp"

namespace opossum {

// TODO(moritz): Auxiliary code. Keep in src/test?
class MockTableNode: public AbstractASTNode {
 public:
  MockTableNode(const std::shared_ptr<TableStatistics> & statistics);

  std::string description() const override;
};

}