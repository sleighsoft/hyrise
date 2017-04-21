#pragma once

#include <memory>
#include <string>
#include <vector>

#include "abstract_read_only_operator.hpp"

namespace opossum {
// operator to create a GroupKey Index on a certain table and column
class CreateIndex : public AbstractReadOnlyOperator {
 public:
  explicit CreateIndex(const std::shared_ptr<const AbstractOperator> in, const std::string& table_name,
                       const std::string& column_name);

  const std::string name() const override;
  uint8_t num_in_tables() const override;
  uint8_t num_out_tables() const override;

 protected:
  std::shared_ptr<const Table> on_execute() override;

  // column and table name to build the index on
  const std::string _table_name;
  const std::string _column_name;
};
}  // namespace opossum
