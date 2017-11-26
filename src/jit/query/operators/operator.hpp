#pragma once

#include <memory>
#include <string>

typedef union {
  uint32_t u32;
  int32_t i32;
  uint64_t u64;
  int64_t i64;
  bool b;
  double d;
  float f;
} Variant;

typedef Variant Tuple[3];

class Operator {
 public:
  using Ptr = std::shared_ptr<const Operator>;

  explicit Operator(const Operator::Ptr& parent) : _parent(parent), _index(parent ? parent->index() + 1 : 0) {}
  virtual ~Operator() = default;

  uint32_t index() const { return _index; }
  std::string name() const { return std::string(typeid(*this).name()); }

  std::string display_name() const {
    const std::string name(typeid(*this).name());
    return name.substr(name.find_first_not_of("0123456789"));
  }

  virtual void next(const Tuple& tuple, uint32_t& result) const = 0;
  __attribute__((noinline)) void emit(const Tuple& tuple, uint32_t& result) const { _parent->next(tuple, result); }

 private:
  const Operator::Ptr _parent;
  const uint32_t _index;
};
