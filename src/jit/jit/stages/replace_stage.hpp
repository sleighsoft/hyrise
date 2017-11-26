#pragma once

#include <boost/filesystem.hpp>

#include "llvm_stage.hpp"

class ReplaceStage : public LLVMStage<void> {
 public:
  explicit ReplaceStage(const boost::filesystem::path& path);

 private:
  std::string name() const final;
  nlohmann::json options() const final;
  void run_impl(std::unique_ptr<llvm::Module>& module, nlohmann::json& json) const final;

  const boost::filesystem::path _path;
};
