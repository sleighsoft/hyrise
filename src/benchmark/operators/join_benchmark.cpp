#include <memory>

#include "benchmark/benchmark.h"

#include "../benchmark_basic_fixture.hpp"
#include "operators/join_hash.hpp"
#include "operators/join_nested_loop.hpp"
#include "operators/join_sort_merge.hpp"
#include "operators/table_wrapper.hpp"

namespace opossum {

BENCHMARK_DEFINE_F(BenchmarkBasicFixture, BM_NLJ)(benchmark::State& state) {
  clear_cache();

  auto warm_up = std::make_shared<JoinNestedLoop>(_table_wrapper_a, _table_wrapper_b, JoinMode::Inner,
                                                  std::make_pair(ColumnID{0}, ColumnID{0}), ScanType::OpEquals);
  warm_up->execute();

  while (state.KeepRunning()) {
    auto join = std::make_shared<JoinNestedLoop>(_table_wrapper_a, _table_wrapper_b, JoinMode::Inner,
                                                 std::make_pair(ColumnID{0}, ColumnID{0}), ScanType::OpEquals);
    join->execute();
  }
}

BENCHMARK_DEFINE_F(BenchmarkBasicFixture, BM_SMJ)(benchmark::State& state) {
  clear_cache();

  auto warm_up = std::make_shared<JoinSortMerge>(_table_wrapper_a, _table_wrapper_b, JoinMode::Inner,
                                                 std::make_pair(ColumnID{0}, ColumnID{0}), ScanType::OpEquals);
  warm_up->execute();

  while (state.KeepRunning()) {
    auto join = std::make_shared<JoinSortMerge>(_table_wrapper_a, _table_wrapper_b, JoinMode::Inner,
                                                std::make_pair(ColumnID{0}, ColumnID{0}), ScanType::OpEquals);
    join->execute();
  }
}
BENCHMARK_DEFINE_F(BenchmarkBasicFixture, BM_Hash)(benchmark::State& state) {
  clear_cache();

  auto warm_up = std::make_shared<JoinHash>(_table_wrapper_a, _table_wrapper_b, JoinMode::Inner,
                                            std::make_pair(ColumnID{0}, ColumnID{0}), ScanType::OpEquals);
  warm_up->execute();

  while (state.KeepRunning()) {
    auto join = std::make_shared<JoinHash>(_table_wrapper_a, _table_wrapper_b, JoinMode::Inner,
                                           std::make_pair(ColumnID{0}, ColumnID{0}), ScanType::OpEquals);
    join->execute();
  }
}

BENCHMARK_REGISTER_F(BenchmarkBasicFixture, BM_NLJ)->Apply(BenchmarkBasicFixture::ChunkSizeIn);
BENCHMARK_REGISTER_F(BenchmarkBasicFixture, BM_SMJ)->Apply(BenchmarkBasicFixture::ChunkSizeIn);
BENCHMARK_REGISTER_F(BenchmarkBasicFixture, BM_Hash)->Apply(BenchmarkBasicFixture::ChunkSizeIn);

}  // namespace opossum
