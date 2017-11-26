#include "replace_stage.hpp"

#include "../../util/error_utils.hpp"
#include "../../util/file_utils.hpp"
#include "../../util/llvm_utils.hpp"

ReplaceStage::ReplaceStage(const boost::filesystem::path& path) : _path(path) {}

std::string ReplaceStage::name() const { return "replace"; }

nlohmann::json ReplaceStage::options() const { return {{"filename", _path.filename().string()}}; }

void ReplaceStage::run_impl(std::unique_ptr<llvm::Module>& module, nlohmann::json& json) const {
  module = llvm_utils::module_from_file(_path, module->getContext());
}
