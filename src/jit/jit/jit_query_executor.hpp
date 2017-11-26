#pragma once

#include <cstdint>

#include <llvm/IR/LLVMContext.h>

#include "../query/query.hpp"
#include "jit.hpp"

class JitQueryExecutor {
 public:
  JitQueryExecutor();
  void run_query(const Query& query) const;

 private:
  const std::shared_ptr<llvm::LLVMContext> _context;
};
