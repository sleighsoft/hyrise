#pragma once

#include <cstdlib>
#include <string>
#include <vector>

#include "source.hpp"

class GetTable : public Source {
 public:
  GetTable(const Operator::Ptr& parent, size_t num_rows, uint32_t seed);

  void run(uint32_t& result) const final;

 private:
  std::vector<uint32_t> _column;
};
