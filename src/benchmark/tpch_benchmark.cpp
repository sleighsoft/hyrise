#include <iostream>
#include <string>

#include "benchmark/benchmark.h"

#include "concurrency/transaction_manager.hpp"
#include "tpch/tpch_queries.hpp"
#include "tpch/tpch_table_generator.hpp"
#include "storage/storage_manager.hpp"
#include "sql/sql_planner.hpp"
#include "scheduler/current_scheduler.hpp"

#include "SQLParser.h"
#include "SQLParserResult.h"

namespace opossum {

class TPCHBenchmark : public ::benchmark::Fixture {
 static bool _tpch_tables_generated;

 public:
  void SetUp(benchmark::State& state) override {
    if (!_tpch_tables_generated) {
      const auto tpch_tables_by_name = tpch::TpchTableGenerator().generate_all_tables();

      for (const auto& name_and_table : tpch_tables_by_name) {
        StorageManager::get().add_table(name_and_table.first, name_and_table.second);
      }

      _tpch_tables_generated = true;
    }

  }
};

bool TPCHBenchmark::_tpch_tables_generated = false;

BENCHMARK_DEFINE_F(TPCHBenchmark, TPCHQuery)(benchmark::State& state) {
  const auto& query_c_str = tpch_queries[state.range(0)];

  /**
   * TODO(anybody): All-over-hyrise redundant SQL handling code
   */
  hsql::SQLParserResult parser_result;
  hsql::SQLParser::parse(query_c_str, &parser_result);

  if (!parser_result.isValid()) {
    state.SkipWithError((std::string("Invalid SQL: ") + query_c_str).c_str());
    return;
  }

  auto plan = SQLPlanner::plan(parser_result);

  if (plan.tree_roots().size() != 1) {
    state.SkipWithError((std::string("Invalid number of tree roots generated: ") + std::to_string(plan.tree_roots().size())).c_str());
    return;
  }

  while (state.KeepRunning()) {
    for (const auto& root : plan.tree_roots()) {
      auto tasks = OperatorTask::make_tasks_from_operator(root);
      CurrentScheduler::schedule_and_wait_for_tasks(tasks);
    }
  }
}

BENCHMARK_REGISTER_F(TPCHBenchmark, TPCHQuery)
  ->Arg(0)->Arg(2)->Arg(4)->Arg(5)->Arg(8)->Arg(9);

}