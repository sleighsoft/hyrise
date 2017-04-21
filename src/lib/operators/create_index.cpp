#include "create_index.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "storage/index/group_key/group_key_index.hpp"
#include "storage/storage_manager.hpp"

namespace opossum {

CreateIndex::CreateIndex(const std::shared_ptr<const AbstractOperator> in, const std::string& table_name,
                         const std::string& column_name)
    : AbstractReadOnlyOperator(in), _table_name(table_name), _column_name(column_name) {}

const std::string CreateIndex::name() const { return "CreateIndex"; }

uint8_t CreateIndex::num_in_tables() const { return 1; }

uint8_t CreateIndex::num_out_tables() const { return 1; }

std::shared_ptr<const Table> CreateIndex::on_execute() {
  auto in_table = StorageManager::get().get_table(_table_name);
  ColumnID column_id = in_table->column_id_by_name(_column_name);

  for (ChunkID chunk_id = 0; chunk_id < in_table->chunk_count(); ++chunk_id) {
    auto& chunk_in = in_table->get_chunk(chunk_id);
    chunk_in.create_index<GroupKeyIndex>({chunk_in.get_column(column_id)});
  }

  return input_table_left();
}

}  // namespace opossum
