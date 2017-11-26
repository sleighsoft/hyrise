#include "get_table_mock.hpp"

GetTableMock::GetTableMock(const Operator::Ptr& parent) : Source(parent) {}

void GetTableMock::run(uint32_t& result) const {
  Tuple tuple;
  tuple[0].u32 = 10;
  emit(tuple, result);
  tuple[0].u32 = 20;
  emit(tuple, result);
  tuple[0].u32 = 30;
  emit(tuple, result);
}
