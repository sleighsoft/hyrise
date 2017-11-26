#include "extract_stage.hpp"

#include <unordered_set>

#include <llvm/ADT/SetVector.h>
#include <llvm/Support/Regex.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Instructions.h>

#include "../../util/error_utils.hpp"
#include "../../util/file_utils.hpp"
#include "../../util/llvm_utils.hpp"

ExtractStage::ExtractStage(const std::initializer_list<std::string> &patterns, const bool recursive)
    : _patterns(patterns), _recursive(recursive) {}

std::string ExtractStage::name() const { return "llvm-extract"; }

nlohmann::json ExtractStage::options() const { return {{"patterns", _patterns}, {"recursive", _recursive}}; }

void ExtractStage::run_impl(std::unique_ptr<llvm::Module> &module, nlohmann::json &json) const {

  std::vector<llvm::Function *> queue;

  // extract functions via regular expression matching.
  for (const auto &pattern : _patterns) {
    llvm::Regex regex(pattern);
    for (auto &function : *module) {
      if (regex.match(function.getName())) {
        queue.push_back(&function);
      }
    }
  }


  std::unordered_set<llvm::Function*> visited;

  while (!queue.empty()) {
    llvm::Function *function = queue.back();
    queue.pop_back();
    error_utils::handle_error(function->materialize());
    visited.insert(function);

    if (_recursive) {
      for (auto &block : *function) {
        for (auto &inst : block) {
          if (auto *call_inst = llvm::dyn_cast<llvm::CallInst>(&inst)) {
            llvm::Function *called_function = call_inst->getCalledFunction();
            if (called_function && !called_function->isDeclaration() && !visited.count(called_function)) {
              queue.push_back(called_function);
            }
          }
        }
      }
    }
  }

  std::vector<llvm::GlobalValue *> globals(visited.begin(), visited.end());

  llvm::legacy::PassManager extract_pass;
  extract_pass.add(llvm::createGVExtractionPass(globals, false));
  extract_pass.run(*module);
  error_utils::handle_error(module->materializeAll());

  llvm::legacy::PassManager cleanup_pass;
  cleanup_pass.add(llvm::createGlobalDCEPass());             // delete unreachable globals
  cleanup_pass.add(llvm::createStripDeadDebugInfoPass());    // remove dead debug info
  cleanup_pass.add(llvm::createStripDeadPrototypesPass());   // remove dead func decls
  cleanup_pass.run(*module);
}
