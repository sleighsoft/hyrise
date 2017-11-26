#include "table_scan.hpp"

TableScan::TableScan(const Operator::Ptr& parent, TableScan::CompOperator op, uint32_t value)
    : Operator(parent), _op(op), _value(value) {}

void TableScan::next(const Tuple& tuple, uint32_t& result) const {
  switch (_op) {
    case GreaterThan:
      if (tuple[0].u32 > _value) emit(tuple, result);
      break;
    case LessThan:
      if (tuple[0].u32 < _value) emit(tuple, result);
      break;
    case Equal:
      if (tuple[0].u32 == _value) emit(tuple, result);
      break;
  }
}
