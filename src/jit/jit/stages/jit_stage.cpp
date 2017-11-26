#include "jit_stage.hpp"

JitStage::JitStage(const std::shared_ptr<Jit>& jit) : _jit(jit) {}

std::string JitStage::name() const { return "jit compile"; }

void JitStage::run_impl(std::unique_ptr<llvm::Module>& module, nlohmann::json& json) const {}
