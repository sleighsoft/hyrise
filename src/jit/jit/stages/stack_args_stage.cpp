#include <unordered_map>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>

#include "stack_args_stage.hpp"

std::string StackArgsStage::name() const { return "stack-args"; }

void StackArgsStage::run_impl(std::unique_ptr<llvm::Module>& module, nlohmann::json& json) const {
  for (auto& function : *module) {
    if (function.isDeclaration()) {
      continue;
    }

    std::unordered_map<llvm::Argument*, llvm::Value*> stack_args;
    llvm::IRBuilder<> builder(&function.front().front());

    for (auto& arg : function.args()) {
      stack_args[&arg] = builder.CreateAlloca(arg.getType()->getPointerElementType());
      arg.replaceAllUsesWith(stack_args[&arg]);
      auto value = builder.CreateLoad(&arg);
      builder.CreateStore(value, stack_args[&arg]);
    }

    for (auto& block : function) {
      if (llvm::isa<llvm::ReturnInst>(block.back())) {
        for (auto& arg : function.args()) {
          llvm::IRBuilder<> return_builder(&block.back());
          auto value = return_builder.CreateLoad(stack_args[&arg]);
          return_builder.CreateStore(value, &arg);
        }
      }
    }
  }
}
