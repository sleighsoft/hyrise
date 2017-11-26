#pragma once

#include "llvm_stage.hpp"

class OptStage : public LLVMStage<void> {
 public:
  enum OptimizationLevel { O0 = 0, O1 = 1, O2 = 2, O3 = 3 };

  OptStage(const std::string &binary, const OptimizationLevel level, const uint32_t inline_threshold);

 private:
  std::string name() const final;
  nlohmann::json options() const final;
  void run_impl(std::unique_ptr<llvm::Module>& module, nlohmann::json& json) const final;

  const std::string _binary;
  const OptimizationLevel _level;
  const uint32_t _inline_threshold;
};
