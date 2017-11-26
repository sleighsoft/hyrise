#pragma once

#include <boost/filesystem.hpp>

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/FileSystem.h>

struct llvm_utils {
  static std::string module_to_string(const llvm::Module& module) {
    std::string content;
    llvm::raw_string_ostream os(content);
    module.print(os, nullptr, false, true);
    return content;
  }

  static void module_to_file(const boost::filesystem::path& path, const llvm::Module& module) {
    std::error_code error_code;
    llvm::raw_fd_ostream os(path.string(), error_code, llvm::sys::fs::F_None);
    llvm::WriteBitcodeToFile(&module, os);
    os.flush();
    error_utils::handle_error(error_code.value());
  }

  static std::unique_ptr<llvm::Module> module_from_file(const boost::filesystem::path& path,
                                                        llvm::LLVMContext& context) {
    llvm::SMDiagnostic error;
    auto module = llvm::parseIRFile(path.string(), error, context);
    error_utils::handle_error(error);
    return module;
  }
};
