#pragma once

#include <llvm/Target/TargetMachine.h>

#include "./llvm_stage.hpp"

class ConstantSubstituteStage : public LLVMStage<Operator> {
 private:
  std::string name() const final;
  void run_impl(std::unique_ptr<llvm::Module>& module, const Operator& op, nlohmann::json& json) const final;
};
