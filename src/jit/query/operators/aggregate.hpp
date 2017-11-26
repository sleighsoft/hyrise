#pragma once

#include <cstdint>
#include <string>

#include "operator.hpp"

class Aggregate : public Operator {
 public:
  enum Function { Count, Sum };

  Aggregate(const Operator::Ptr& parent, Function fn);

  void next(const Tuple& tuple, uint32_t& result) const final;

 private:
  const Function _fn;
};
