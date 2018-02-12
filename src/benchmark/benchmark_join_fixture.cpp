#include "benchmark_join_fixture.hpp"

#include <memory>
#include <vector>

#include "benchmark/benchmark.h"
#include "operators/table_wrapper.hpp"
#include "storage/chunk.hpp"
#include "storage/index/adaptive_radix_tree/adaptive_radix_tree_index.hpp"
#include "storage/index/group_key/composite_group_key_index.hpp"
#include "storage/storage_manager.hpp"
#include "table_generator.hpp"
#include "types.hpp"

namespace {
enum class BenchmarkScenario { Normal, Uniform, Pareto, Disjunct, CompositeGroupKeyIndex };
}

namespace opossum {

void BenchmarkJoinFixture::SetUp(::benchmark::State& state) {
  // Generating a test table with generate_table function from table_generator.cpp

  auto table_generator = std::make_shared<TableGenerator>();

  BenchmarkScenario benchmark_scenario = static_cast<BenchmarkScenario>(state.range(2));

  auto left_config = ColumnConfiguration::make_uniform_config(0, 10000);

  ColumnConfiguration right_config;

  switch (benchmark_scenario) {
    case BenchmarkScenario::Normal:
      right_config = ColumnConfiguration::make_skewed_normal_config();
      break;
    case BenchmarkScenario::Pareto:
      right_config = ColumnConfiguration::make_pareto_config();
      break;
    case BenchmarkScenario::Uniform:
      right_config = ColumnConfiguration::make_uniform_config(0, 10000);
      break;
    case BenchmarkScenario::Disjunct:
      right_config = ColumnConfiguration::make_uniform_config(20000, 30000);
      break;
    case BenchmarkScenario::CompositeGroupKeyIndex:
      right_config = ColumnConfiguration::make_uniform_config(0, 10000);
      break;
  }

  auto table_1 = table_generator->generate_table(std::vector<ColumnConfiguration>{left_config}, state.range(0),
                                                 state.range(0) / 4, true);
  auto table_2 = table_generator->generate_table(std::vector<ColumnConfiguration>{right_config}, state.range(1),
                                                 state.range(1) / 4, true);

  for (auto table : {table_1, table_2}) {
    for (ChunkID chunk_id{0}; chunk_id < table->chunk_count(); ++chunk_id) {
      auto chunk = table->get_chunk(chunk_id);

      std::vector<ColumnID> columns{1};
      for (ColumnID column_id{0}; column_id < chunk->column_count(); ++column_id) {
        columns[0] = column_id;

        if (benchmark_scenario == BenchmarkScenario::CompositeGroupKeyIndex) {
          chunk->create_index<CompositeGroupKeyIndex>(columns);
        } else {
          chunk->create_index<AdaptiveRadixTreeIndex>(columns);
        }
      }
    }
  }

  _tw_small_uni1 = std::make_shared<TableWrapper>(table_1);
  _tw_small_uni2 = std::make_shared<TableWrapper>(table_2);
  _tw_small_uni1->execute();
  _tw_small_uni2->execute();
}

void BenchmarkJoinFixture::TearDown(::benchmark::State&) { opossum::StorageManager::get().reset(); }

void BenchmarkJoinFixture::ChunkSizeInUni(benchmark::internal::Benchmark* b) {
  for (int left_size : {100, 1000, 10000, 100000, 1000000}) {
    for (int right_size : {100, 1000, 10000, 100000, 1000000, 5000000}) {
      // make sure we do not overrun our memory capacity
      if (static_cast<uint64_t>(left_size) * static_cast<uint64_t>(right_size) <= 1e9) {
        b->Args({left_size, right_size,
                 static_cast<int>(BenchmarkScenario::Uniform)});  // left size, right size, BenchmarkScenario
      }
    }
  }
}

void BenchmarkJoinFixture::ChunkSizeInNormal(benchmark::internal::Benchmark* b) {
  for (int left_size : {100, 1000, 10000, 100000, 1000000}) {
    for (int right_size : {100, 1000, 10000, 100000, 1000000, 5000000}) {
      // make sure we do not overrun our memory capacity
      if (static_cast<uint64_t>(left_size) * static_cast<uint64_t>(right_size) <= 1e9) {
        b->Args({left_size, right_size,
                 static_cast<int>(BenchmarkScenario::Normal)});  // left size, right size, BenchmarkScenario
      }
    }
  }
}
void BenchmarkJoinFixture::ChunkSizeInPareto(benchmark::internal::Benchmark* b) {
  for (int left_size : {100, 1000, 10000, 100000, 1000000}) {
    for (int right_size : {100, 1000, 10000, 100000, 1000000, 5000000}) {
      // make sure we do not overrun our memory capacity
      if (static_cast<uint64_t>(left_size) * static_cast<uint64_t>(right_size) <= 1e9) {
        b->Args({left_size, right_size,
                 static_cast<int>(BenchmarkScenario::Pareto)});  // left size, right size, BenchmarkScenario
      }
    }
  }
}

void BenchmarkJoinFixture::ChunkSizeInDisjunct(benchmark::internal::Benchmark* b) {
  for (int left_size : {100, 1000, 10000, 100000, 1000000}) {
    for (int right_size : {100, 1000, 10000, 100000, 1000000, 5000000}) {
      // make sure we do not overrun our memory capacity
      if (static_cast<uint64_t>(left_size) * static_cast<uint64_t>(right_size) <= 1e9) {
        b->Args({left_size, right_size,
                 static_cast<int>(BenchmarkScenario::Disjunct)});  // left size, right size, BenchmarkScenario
      }
    }
  }
}

void BenchmarkJoinFixture::ChunkSizeInCompIndex(benchmark::internal::Benchmark* b) {
  for (int left_size : {100, 1000, 10000, 100000, 1000000}) {
    for (int right_size : {100, 1000, 10000, 100000, 1000000, 5000000}) {
      // make sure we do not overrun our memory capacity
      if (static_cast<uint64_t>(left_size) * static_cast<uint64_t>(right_size) <= 1e9) {
        b->Args({left_size, right_size,
                 static_cast<int>(BenchmarkScenario::CompositeGroupKeyIndex)});  // left size, right size, BenchmarkScenario
      }
    }
  }
}

void BenchmarkJoinFixture::clear_cache() {
  std::vector<int> clear = std::vector<int>();
  clear.resize(500 * 1000 * 1000, 42);
  for (uint i = 0; i < clear.size(); i++) {
    clear[i] += 1;
  }
  clear.resize(0);
}

}  // namespace opossum
