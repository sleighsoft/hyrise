#include "print.hpp"
#include "operator.hpp"

#include <iostream>

Print::Print(const Operator::Ptr& parent, std::ostream& os) : Operator(parent), _os(os) {}

void Print::next(const Tuple& tuple, uint32_t& result) const {
  _os << tuple[0].u32 << " " << tuple[1].b << tuple[2].d << " " << std::endl;
}
