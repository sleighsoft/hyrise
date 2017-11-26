#pragma once

#include "./llvm_stage.hpp"

class ManualInlineStage : public LLVMStage<Query> {
 private:
  std::string name() const final;
  void run_impl(std::unique_ptr<llvm::Module>& module, const Query& query, nlohmann::json& json) const final;
};
