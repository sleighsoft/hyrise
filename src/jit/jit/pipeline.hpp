#pragma once

#include <chrono>
#include <cstdlib>
#include <vector>

#include "../util/llvm_utils.hpp"
#include "stages/llvm_stage.hpp"

template <typename T>
class Pipeline {
 public:
  Pipeline(std::initializer_list<std::shared_ptr<LLVMStage<T>>> stages) : _stages(stages) {}

  void run(std::unique_ptr<llvm::Module>& module, const T& context, nlohmann::json& json) {
    boost::filesystem::path before_path, after_path;
    std::string module_string;

    std::tie(module_string, before_path) = dump_module(*module);
    json.push_back(
        {{"type", "asset"}, {"language", "llvm"}, {"content", module_string}, {"name", before_path.string()}});

    for (const auto& stage : _stages) {
      nlohmann::json stage_json = {{"analysis", nlohmann::json::array()}};
      stage->run(module, context, stage_json);
      std::tie(module_string, after_path) = dump_module(*module);

      stage_json["analysis"].push_back(diff(before_path, after_path));
      stage_json["analysis"].push_back(llvm_diff(before_path, after_path));
      json.push_back(stage_json);
      json.push_back(
          {{"type", "asset"}, {"language", "llvm"}, {"content", module_string}, {"name", after_path.string()}});
      before_path = after_path;
    }
  }

 private:
  std::pair<std::string, boost::filesystem::path> dump_module(const llvm::Module& module) const {
    boost::filesystem::path path = file_utils::temp_file("ll");
    std::string module_string = llvm_utils::module_to_string(module);
    file_utils::write_to_file(path, module_string);
    return std::make_pair(module_string, path);
  }

  nlohmann::json diff(const boost::filesystem::path& before_path, const boost::filesystem::path& after_path) const {
    auto diff_path = file_utils::temp_file("diff");

    std::ostringstream command;
    command << "diff -Naur " << before_path << " " << after_path << " > " << diff_path << " 2> /dev/null";
    auto exit_code = system(command.str().c_str());
    assert(exit_code == 0 || exit_code == 256);
    return {{"type", "diff"}, {"name", diff_path.string()}, {"content", file_utils::read_from_file(diff_path)}};
  }

  nlohmann::json llvm_diff(const boost::filesystem::path& before_path,
                           const boost::filesystem::path& after_path) const {
    auto diff_path = file_utils::temp_file("llvm_diff");

    std::ostringstream command;
    command << "llvm-diff " << before_path << " " << after_path << " 2> " << diff_path << " > /dev/null";
    auto exit_code = system(command.str().c_str());
    assert(exit_code == 0 || exit_code == 256);
    return {{"type", "llvm-diff"}, {"name", diff_path.string()}, {"content", file_utils::read_from_file(diff_path)}};
  }

  std::vector<std::shared_ptr<LLVMStage<T>>> _stages;
};
