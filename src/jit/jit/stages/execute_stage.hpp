#pragma once

#include <json.hpp>

#include <llvm/IR/Module.h>

#include "../../query/query.hpp"
#include "stage.hpp"

class ExecuteStage : public Stage<std::unique_ptr<llvm::Module>, const Query> {
 //private:
  std::string name() const final;
  void run_impl(std::unique_ptr<llvm::Module>& module, const Query& context, nlohmann::json& json) const;
};
