#pragma once

#include "./llvm_stage.hpp"

class RenameStage : public LLVMStage<Operator> {
 public:
  explicit RenameStage(const std::string& prefix);

 private:
  std::string name() const final;
  nlohmann::json options() const final;
  void run_impl(std::unique_ptr<llvm::Module>& module, const Operator& op, nlohmann::json& json) const final;

  const std::string _prefix;
};
