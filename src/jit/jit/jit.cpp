#include "jit.hpp"

#include <llvm/ExecutionEngine/RTDyldMemoryManager.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>

#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

Jit::Jit(std::shared_ptr<llvm::LLVMContext> context)
    : _context(std::move(context)),
      _target_machine(!llvm::InitializeNativeTarget() && !llvm::InitializeNativeTargetAsmPrinter()
                          ? llvm::EngineBuilder().selectTarget()
                          : nullptr),
      _data_layout(_target_machine->createDataLayout()),
      _object_layer([]() { return std::make_shared<llvm::SectionMemoryManager>(); }),
      _compile_layer(_object_layer, llvm::orc::SimpleCompiler(*_target_machine)) {
  // make exported symbols of the current process available for execution
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

Jit::ModuleHandle Jit::add_module(std::shared_ptr<llvm::Module> module) {
  module->setDataLayout(_data_layout);
  auto resolver = llvm::orc::createLambdaResolver(
      [this](const std::string& name) { return _compile_layer.findSymbol(name, true); },
      [](const std::string& name) {
        return llvm::JITSymbol(llvm::RTDyldMemoryManager::getSymbolAddressInProcess(name),
                               llvm::JITSymbolFlags::Exported);
      });

  auto handle = error_utils::handle_error(_compile_layer.addModule(std::move(module), std::move(resolver)));
  _modules.push_back(handle);
  return handle;
}

void Jit::remove_module(const Jit::ModuleHandle& handle) {
  _modules.erase(std::find(_modules.cbegin(), _modules.cend(), handle));
  error_utils::handle_error(_compile_layer.removeModule(handle));
}
