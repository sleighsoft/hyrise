#pragma once

#include "../jit.hpp"
#include "stage.hpp"

class JitStage : public Stage<std::unique_ptr<llvm::Module>> {
 public:
  explicit JitStage(const std::shared_ptr<Jit>& jit);

 private:
  std::string name() const final;
  void run_impl(std::unique_ptr<llvm::Module>& module, nlohmann::json& json) const final;

  const std::shared_ptr<Jit> _jit;
};
