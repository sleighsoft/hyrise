#pragma once

#include "operator.hpp"

class Source : public Operator {
 public:
  using Ptr = std::shared_ptr<const Source>;

  explicit Source(const Operator::Ptr& parent) : Operator(parent) {}
  virtual ~Source() = default;

  virtual void run(uint32_t& result) const = 0;

 private:
  void next(const Tuple& tuple, uint32_t& result) const final {}
};
