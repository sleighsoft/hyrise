#pragma once

#include "llvm_stage.hpp"

class ExtractStage : public LLVMStage<void> {
 public:
  ExtractStage(const std::initializer_list<std::string>& patterns, const bool recursive);

 private:
  std::string name() const final;
  nlohmann::json options() const final;
  void run_impl(std::unique_ptr<llvm::Module>& module, nlohmann::json& json) const final;

  const std::vector<std::string> _patterns;
  const bool _recursive;
};
