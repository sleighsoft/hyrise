#include "strip_debug_info_stage.hpp"

#include <llvm/IR/DebugInfo.h>

std::string StripDebugInfoStage::name() const { return "strip-debug"; }

void StripDebugInfoStage::run_impl(std::unique_ptr<llvm::Module>& module, nlohmann::json& json) const {
  llvm::StripDebugInfo(*module);
}
