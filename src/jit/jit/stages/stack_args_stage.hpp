#pragma once

#include "llvm_stage.hpp"

class StackArgsStage : public LLVMStage<void> {
 private:
  std::string name() const final;
  void run_impl(std::unique_ptr<llvm::Module>& module, nlohmann::json& json) const final;
};
