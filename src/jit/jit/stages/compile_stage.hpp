#pragma once

#include <llvm/IR/Module.h>

#include "stage.hpp"

class CompileStage : public Stage<const std::string, std::unique_ptr<llvm::Module>> {
 public:
  enum OptimizationLevel { O0 = 0, O1 = 1, O2 = 2, O3 = 3 };

  CompileStage(const std::string& binary, const OptimizationLevel level, const std::string& standard,
               const std::shared_ptr<llvm::LLVMContext>& context);

 private:
  std::string name() const final;
  nlohmann::json options() const final;
  void run_impl(const std::string& source, std::unique_ptr<llvm::Module>& module, nlohmann::json& json) const final;

  const std::string _binary;
  const OptimizationLevel _level;
  const std::string _standard;
  const std::shared_ptr<llvm::LLVMContext> _context;
};
