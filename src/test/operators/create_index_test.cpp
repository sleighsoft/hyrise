#include <memory>
#include <utility>

#include "../base_test.hpp"
#include "gtest/gtest.h"

#include "../../lib/operators/create_index.hpp"
#include "../../lib/operators/get_table.hpp"
#include "../../lib/storage/storage_manager.hpp"
#include "../../lib/storage/table.hpp"

namespace opossum {
// The fixture for testing class GetTable.
class OperatorsCreateIndexTest : public BaseTest {
 protected:
  void SetUp() override {
    std::shared_ptr<Table> test_table = load_table("src/test/tables/int_float.tbl", 0);
    StorageManager::get().add_table("table_a", std::move(test_table));

    StorageManager::get().get_table("table_a")->compress_chunk(0);

    _gt = std::make_shared<GetTable>("table_a");
    _gt->execute();
  }

  std::shared_ptr<GetTable> _gt;
};

TEST_F(OperatorsCreateIndexTest, CreateIndex) {
  auto ci = std::make_shared<CreateIndex>(_gt, "table_a", "b");
  ci->execute();

  auto& chunk = ci->get_output()->get_chunk(0);
  auto column_a = chunk.get_column(0);
  auto column_b = chunk.get_column(1);

  EXPECT_EQ(chunk.get_indices_for({column_a}).size(), (size_t)0);
  EXPECT_EQ(chunk.get_indices_for({column_b}).size(), (size_t)1);
}

}  // namespace opossum
