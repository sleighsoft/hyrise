#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../base_test.hpp"
#include "gtest/gtest.h"

#include "concurrency/transaction_context.hpp"
#include "concurrency/transaction_manager.hpp"
#include "operators/delete.hpp"
#include "operators/get_table.hpp"
#include "operators/table_scan.hpp"
#include "operators/update.hpp"
#include "operators/validate.hpp"
#include "storage/storage_manager.hpp"
#include "storage/table.hpp"
#include "types.hpp"

namespace opossum {

class OperatorsDeleteTest : public BaseTest {
 protected:
  void SetUp() override {
    _table_name = "table_a";
    _table = load_table("src/test/tables/float_int.tbl", Chunk::MAX_SIZE);
    // Delete Operator works with the Storage Manager, so the test table must also be known to the StorageManager
    StorageManager::get().add_table(_table_name, _table);
    _gt = std::make_shared<GetTable>(_table_name);

    _gt->execute();
  }

  std::string _table_name;
  std::shared_ptr<GetTable> _gt;
  std::shared_ptr<Table> _table;

  void helper(bool commit);
};

void OperatorsDeleteTest::helper(bool commit) {
  auto transaction_context = TransactionManager::get().new_transaction_context();

  // Selects two out of three rows.
  auto table_scan = std::make_shared<TableScan>(_gt, ColumnID{0}, ScanType::OpGreaterThan, "456.7");

  table_scan->execute();

  auto delete_op = std::make_shared<Delete>(_table_name, table_scan);
  delete_op->set_transaction_context(transaction_context);

  delete_op->execute();

  EXPECT_EQ(_table->get_chunk(ChunkID{0}).mvcc_columns()->tids.at(0u), transaction_context->transaction_id());
  EXPECT_EQ(_table->get_chunk(ChunkID{0}).mvcc_columns()->tids.at(1u), 0u);
  EXPECT_EQ(_table->get_chunk(ChunkID{0}).mvcc_columns()->tids.at(2u), transaction_context->transaction_id());

  // Table has three rows initially.
  EXPECT_EQ(_table->approx_valid_row_count(), 3u);

  auto expected_end_cid = CommitID{0u};
  if (commit) {
    transaction_context->commit();
    expected_end_cid = transaction_context->commit_id();

    // Delete successful, one row left.
    EXPECT_EQ(_table->approx_valid_row_count(), 1u);
  } else {
    transaction_context->rollback();
    expected_end_cid = Chunk::MAX_COMMIT_ID;

    // Delete rolled back, three rows left.
    EXPECT_EQ(_table->approx_valid_row_count(), 3u);
  }

  EXPECT_EQ(_table->get_chunk(ChunkID{0}).mvcc_columns()->end_cids.at(0u), expected_end_cid);
  EXPECT_EQ(_table->get_chunk(ChunkID{0}).mvcc_columns()->end_cids.at(1u), Chunk::MAX_COMMIT_ID);
  EXPECT_EQ(_table->get_chunk(ChunkID{0}).mvcc_columns()->end_cids.at(2u), expected_end_cid);

  auto expected_tid = commit ? transaction_context->transaction_id() : 0u;

  EXPECT_EQ(_table->get_chunk(ChunkID{0}).mvcc_columns()->tids.at(0u), expected_tid);
  EXPECT_EQ(_table->get_chunk(ChunkID{0}).mvcc_columns()->tids.at(1u), 0u);
  EXPECT_EQ(_table->get_chunk(ChunkID{0}).mvcc_columns()->tids.at(2u), expected_tid);
}


}  // namespace opossum
