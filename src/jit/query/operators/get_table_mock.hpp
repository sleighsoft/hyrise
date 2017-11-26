#pragma once

#include <string>

#include "source.hpp"

class GetTableMock : public Source {
 public:
  explicit GetTableMock(const Operator::Ptr& parent);

  void run(uint32_t& result) const final;
};
