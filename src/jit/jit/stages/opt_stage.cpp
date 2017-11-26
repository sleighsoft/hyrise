#include "opt_stage.hpp"

#include "../../util/error_utils.hpp"
#include "../../util/file_utils.hpp"
#include "../../util/llvm_utils.hpp"

OptStage::OptStage(const std::string &binary, const OptimizationLevel level, const uint32_t inline_threshold)
    : _binary(binary), _level(level), _inline_threshold(inline_threshold) {}

std::string OptStage::name() const { return "opt"; }

nlohmann::json OptStage::options() const {
  return {{"optimizationLevel", "O" + std::to_string(_level)}, {"inlineThreshold", _inline_threshold}};
}

void OptStage::run_impl(std::unique_ptr<llvm::Module>& module, nlohmann::json& json) const {
  auto before_path = file_utils::temp_file("bc");
  auto after_path = file_utils::temp_file("bc");
  auto remarks_path = file_utils::temp_file("yaml");

  llvm_utils::module_to_file(before_path, *module);

  std::ostringstream command;
  command << _binary << " -inline-threshold=" << _inline_threshold << " -O" << _level << " -o " << after_path << " "
          << before_path << " -pass-remarks-output=" << remarks_path << " > /dev/null 2> /dev/null";
  error_utils::handle_error(system(command.str().c_str()));

  json["analysis"].push_back(
      {{"type", "opt"}, {"name", remarks_path.string()}, {"content", file_utils::read_from_file(remarks_path)}});
  module = llvm_utils::module_from_file(after_path, module->getContext());
}
