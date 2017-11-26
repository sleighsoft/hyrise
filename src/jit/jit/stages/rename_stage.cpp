#include "rename_stage.hpp"

RenameStage::RenameStage(const std::string& prefix) : _prefix(prefix) {}

std::string RenameStage::name() const { return "rename"; }

nlohmann::json RenameStage::options() const { return {{"prefix", _prefix}}; }

void RenameStage::run_impl(std::unique_ptr<llvm::Module>& module, const Operator& op, nlohmann::json& json) const {
  auto prefix = _prefix + std::to_string(op.index());
  for (auto& function : *module) {
    if (!function.isDeclaration()) {
      function.setName(prefix + function.getName());
    }
  }
}
