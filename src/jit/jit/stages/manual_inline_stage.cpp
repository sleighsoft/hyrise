#include "manual_inline_stage.hpp"

#include <llvm/IR/Instructions.h>

#include "../../util/fn_utils.hpp"

std::string ManualInlineStage::name() const { return "manual-inline"; }

void ManualInlineStage::run_impl(std::unique_ptr<llvm::Module> &module, const Query &query,
                                 nlohmann::json &json) const {
  fn_utils::pairwise(
      query.operators().begin(), query.operators().end(),
      [&module](const Operator::Ptr &callee, const Operator::Ptr &caller) {
        auto emit_fn = module->getFunction("op" + std::to_string(caller->index()) + "_ZNK8Operator4emitERA3_K7VariantRj");
        auto next_fn =
            module->getFunction("op" + std::to_string(callee->index()) + "_ZNK" + callee->name() + "4nextERA3_K7VariantRj");
        if (!emit_fn || !next_fn) return;
        for (auto &block : *emit_fn) {
          for (auto &inst : block) {
            if (auto *call_inst = llvm::dyn_cast<llvm::CallInst>(&inst)) {
              auto value =
                  new llvm::BitCastInst(call_inst->getArgOperand(0), next_fn->arg_begin()->getType(), "", call_inst);
              call_inst->setCalledFunction(next_fn);
              call_inst->setOperand(0, value);
            }
          }
        }
      });
}
