#include "allow_inline_stage.hpp"

std::string AllowInlineStage::name() const { return "allow-inline"; }

void AllowInlineStage::run_impl(std::unique_ptr<llvm::Module>& module, nlohmann::json& json) const {
  for (auto& function : *module) {
    function.removeFnAttr(llvm::Attribute::NoInline);
  }
}
