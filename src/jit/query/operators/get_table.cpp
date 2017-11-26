#include "get_table.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>

GetTable::GetTable(const Operator::Ptr& parent, size_t num_rows, uint32_t seed) : Source(parent), _column(num_rows) {
  std::mt19937 generator(seed);
  std::uniform_int_distribution<uint32_t> distribution(0, 200);
  std::generate(_column.begin(), _column.end(), std::bind(distribution, generator));
}

void GetTable::run(uint32_t& result) const {
  Tuple tuple;
  for (const auto v : _column) {
    tuple[0].u32 = v;
    emit(tuple, result);
  }
}
