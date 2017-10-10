#include "mock_table_node.hpp"

namespace opossum {

MockTableNode::MockTableNode(const std::shared_ptr<TableStatistics> & statistics):
  AbstractASTNode(ASTNodeType::Mock)
{
  set_statistics(statistics);
}

std::string MockTableNode::description() const {
  return "MockTable";
}

}