#include "aggregate.hpp"

Aggregate::Aggregate(const Operator::Ptr& parent, Function fn) : Operator(parent), _fn(fn) {}

void Aggregate::next(const Tuple& tuple, uint32_t& result) const {
  switch (_fn) {
    case Count:
      result++;
    case Sum:
      result += tuple[0].u32;
      break;
  }
}
