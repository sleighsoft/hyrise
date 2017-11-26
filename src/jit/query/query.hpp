#pragma once

#include <vector>

#include "operators/operator.hpp"
#include "operators/source.hpp"

class Query {
 public:
  template <typename T, typename... Args>
  void add_operator(Args&&... args) {
    auto parent = _operators.empty() ? nullptr : _operators.back();
    _operators.push_back(std::make_shared<T>(parent, args...));
  }

  const std::vector<Operator::Ptr>& operators() const;

  const Source::Ptr source() const;

 protected:
  std::vector<Operator::Ptr> _operators;
};
