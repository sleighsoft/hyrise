#pragma once

#include <memory>

#include <json.hpp>

#include <llvm/IR/Module.h>

#include "../../query/query.hpp"
#include "./stage.hpp"

template <typename T>
class LLVMStage : public Stage<std::unique_ptr<llvm::Module>, const T> {};

template <>
class LLVMStage<void> : public LLVMStage<Operator>, public LLVMStage<Query> {
 public:
  void run_impl(std::unique_ptr<llvm::Module>& module, const Operator& op, nlohmann::json& json) const final {
    run_impl(module, json);
  }

  void run_impl(std::unique_ptr<llvm::Module>& module, const Query& query, nlohmann::json& json) const final {
    run_impl(module, json);
  }

 private:
  virtual void run_impl(std::unique_ptr<llvm::Module>& module, nlohmann::json& json) const = 0;
};
