#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <random>

#include "jit/jit_query_executor.hpp"
#include "query/operators/aggregate.hpp"
#include "query/operators/get_table.hpp"
#include "query/operators/table_scan.hpp"
#include "query/query.hpp"

int main_1() {
  Query query;
  query.add_operator<Aggregate>(Aggregate::Sum);
  query.add_operator<TableScan>(TableScan::GreaterThan, 10);
  query.add_operator<TableScan>(TableScan::LessThan, 100);
  query.add_operator<GetTable>(100000000, 0);

  uint32_t result = 0;
  auto start = std::chrono::steady_clock::now();
  query.source()->run(result);
  auto time = std::round(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());

  std::cout << "result: " << result << " runtime: " << time << "ms" << std::endl;
  return 0;
}

__attribute__((noinline)) uint32_t run_pipeline(const std::vector<uint32_t>& column) {
  uint32_t sum = 0;
  for (const auto v : column) {
    if (v > 10) {
      if (v < 100) {
        sum += v;
      }
    }
  }

  return sum;
}

int main_2() {
  std::vector<uint32_t> column(100000000);
  std::mt19937 generator(0);
  std::uniform_int_distribution<uint32_t> distribution(0, 200);
  std::generate(column.begin(), column.end(), std::bind(distribution, generator));

  auto start = std::chrono::steady_clock::now();
  auto result = run_pipeline(column);
  auto time = std::round(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
  std::cout << "result: " << result << " runtime: " << time << "ms" << std::endl;

  return 0;
}

int main_3() {
  Query query;
  query.add_operator<Aggregate>(Aggregate::Sum);
  query.add_operator<TableScan>(TableScan::GreaterThan, 10);
  query.add_operator<TableScan>(TableScan::LessThan, 100);
  query.add_operator<GetTable>(100000000, 0);

  JitQueryExecutor executor;
  executor.run_query(query);

  return 0;
}

int main() {
  main_1();
  main_2();
  main_3();
  return 0;
}
