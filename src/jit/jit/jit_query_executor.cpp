#include "jit_query_executor.hpp"

#include <boost/filesystem/fstream.hpp>

#include <json.hpp>

#include <llvm/Linker/Linker.h>

#include "../util/file_utils.hpp"
#include "blob_store.hpp"
#include "pipeline.hpp"
#include "stages/allow_inline_stage.hpp"
#include "stages/compile_stage.hpp"
#include "stages/constant_substitute_stage.hpp"
#include "stages/extract_stage.hpp"
#include "stages/manual_inline_stage.hpp"
#include "stages/opt_stage.hpp"
#include "stages/rename_stage.hpp"
#include "stages/stack_args_stage.hpp"
#include "stages/strip_debug_info_stage.hpp"

JitQueryExecutor::JitQueryExecutor() : _context(std::make_shared<llvm::LLVMContext>()) {}

void JitQueryExecutor::run_query(const Query& query) const {
  nlohmann::json query_json;

  // clang-format off

  Pipeline<Operator> operator_pipeline({
      std::make_shared<ExtractStage>(std::initializer_list<std::string>({".*next", ".*run"}), true),
      std::make_shared<RenameStage>("op"),
      std::make_shared<ConstantSubstituteStage>(),
      std::make_shared<StripDebugInfoStage>()
  });

  Pipeline<Query> composite_pipeline({
      std::make_shared<ManualInlineStage>(),
      std::make_shared<AllowInlineStage>(),
      std::make_shared<OptStage>("opt-5.0", OptStage::O3, 5000),
      std::make_shared<ExtractStage>(std::initializer_list<std::string>({".*run"}), true),
      std::make_shared<StackArgsStage>(),
      std::make_shared<OptStage>("opt-5.0", OptStage::O3, 5000)
  });

  // clang-format on

  auto composite = std::make_unique<llvm::Module>("composite", *_context);
  llvm::Linker linker(*composite);

  CompileStage compiler("clang++-5.0", CompileStage::O3, "c++1z", _context);
  for (const auto& op : query.operators()) {
    nlohmann::json operator_json;
    operator_json["name"] = op->display_name();
    operator_json["pipeline"].push_back({{"type", "asset"},
                                         {"language", "cpp"},
                                         {"content", BlobStore::get(op->name() + "_cpp")},
                                         {"name", op->display_name()}});

    nlohmann::json compiler_json;
    std::unique_ptr<llvm::Module> module;
    compiler.run(BlobStore::get(op->name() + "_processed_cpp"), module, compiler_json);
    operator_json["pipeline"].push_back(compiler_json);

    operator_pipeline.run(module, *op, operator_json["pipeline"]);
    linker.linkInModule(std::move(module), llvm::Linker::Flags::None);

    query_json["operators"].push_back(operator_json);
  }

  composite_pipeline.run(composite, query, query_json["composite"]["pipeline"]);

  // TODO

  Jit jit(_context);
  auto start_compile = std::chrono::steady_clock::now();
  jit.add_module(std::move(composite));
  auto time_compile =
      std::round(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start_compile).count());

  query_json["composite"]["pipeline"].push_back(
      {{"type", "stage"}, {"name", "jit compile"}, {"runtime", time_compile}});

  auto run = jit.find_symbol<void(const Operator*, uint32_t&)>("op3_ZNK8GetTable3runERj");

  uint32_t result = 0;
  auto start = std::chrono::steady_clock::now();
  run(query.source().get(), result);
  auto time = std::round(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());

  query_json["composite"]["pipeline"].push_back({{"type", "stage"}, {"name", "execute"}, {"runtime", time}});
  query_json["composite"]["pipeline"].push_back({{"type", "asset"}, {"language", "result"}, {"content", result}});
  file_utils::write_to_file("query.json", query_json.dump(2));
}
