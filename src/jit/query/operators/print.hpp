#pragma once

#include <cstdint>
#include <string>

#include "operator.hpp"

class Print : public Operator {
 public:
  Print(const Operator::Ptr& parent, std::ostream& os);

  void next(const Tuple& tuple, uint32_t& result) const final;

 private:
  std::ostream& _os;
};
