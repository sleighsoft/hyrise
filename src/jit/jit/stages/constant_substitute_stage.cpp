#include "constant_substitute_stage.hpp"
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/TargetSelect.h>

std::string ConstantSubstituteStage::name() const { return "const-subst"; }

void ConstantSubstituteStage::run_impl(std::unique_ptr<llvm::Module>& module, const Operator& op,
                                       nlohmann::json& json) const {
  for (auto& function : *module) {
    for (auto& block : function) {
      for (auto& inst : block) {
        if (auto* loadInst = llvm::dyn_cast<llvm::LoadInst>(&inst)) {
          if (auto* getElementPtrInst = llvm::dyn_cast<llvm::GetElementPtrInst>(loadInst->getPointerOperand())) {
            llvm::APInt offset(64, 0);
            if (getElementPtrInst->accumulateConstantOffset(module->getDataLayout(), offset) &&
                getElementPtrInst->getPointerOperand() == function.arg_begin()) {
              if (auto type = llvm::dyn_cast<llvm::IntegerType>(loadInst->getType())) {
                auto p =
                    reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(&op) + offset.getLimitedValue());
                loadInst->replaceAllUsesWith(llvm::ConstantInt::get(type, *p));
              }
            }
          }
        }
      }
    }
  }
}
