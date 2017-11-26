#include "execute_stage.hpp"

std::string ExecuteStage::name() const { return "execute"; }

void ExecuteStage::run_impl(std::unique_ptr<llvm::Module>& module, const Query& context, nlohmann::json& json) const {
  // return std::make_shared<ResultAsset>(1234);
}
