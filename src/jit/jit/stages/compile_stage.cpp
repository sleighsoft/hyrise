#include "compile_stage.hpp"

#include "../../util/error_utils.hpp"
#include "../../util/file_utils.hpp"
#include "../../util/llvm_utils.hpp"

CompileStage::CompileStage(const std::string& binary, const OptimizationLevel level, const std::string& standard,
                           const std::shared_ptr<llvm::LLVMContext>& context)
    : _binary(binary), _level(level), _standard(standard), _context(context) {}

std::string CompileStage::name() const { return "compile"; }

nlohmann::json CompileStage::options() const {
  return {{"optimizationLevel", "O" + std::to_string(_level)}, {"languageStandard", _standard}};
}

void CompileStage::run_impl(const std::string& source, std::unique_ptr<llvm::Module>& module,
                            nlohmann::json& json) const {
  auto cpp_path = file_utils::temp_file("cpp");
  auto llvm_path = file_utils::temp_file("bc");
  auto remarks_path = file_utils::temp_file("yaml");

  file_utils::write_to_file(cpp_path, source);

  std::ostringstream command;
  command << _binary << " -std=" << _standard << " -O" << _level << " -emit-llvm -c -o " << llvm_path << " "
          << cpp_path << " -fsave-optimization-record -foptimization-record-file=" << remarks_path
          << " > /dev/null 2> /dev/null";
  error_utils::handle_error(system(command.str().c_str()));

  json["analysis"].push_back(
      {{"type", "opt"}, {"name", remarks_path.string()}, {"content", file_utils::read_from_file(remarks_path)}});
  module = llvm_utils::module_from_file(llvm_path, *_context);

  for (auto &function : *module) {
    function.setComdat(nullptr);
  }
}
