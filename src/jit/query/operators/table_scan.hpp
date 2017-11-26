#pragma once

#include <cstdint>
#include <string>

#include "operator.hpp"

class TableScan : public Operator {
 public:
  enum CompOperator { GreaterThan, Equal, LessThan };

  TableScan(const Operator::Ptr& parent, CompOperator op, uint32_t value);

  void next(const Tuple& tuple, uint32_t& result) const final;

 private:
  const CompOperator _op;
  const uint32_t _value;
};
