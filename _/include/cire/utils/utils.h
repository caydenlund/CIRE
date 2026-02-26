#ifndef CIRE_UTILS_H
#define CIRE_UTILS_H

#include <llvm/IR/Module.h>
#include <memory>
#include <string>

namespace llvm_util {
    std::unique_ptr<llvm::Module> openInputFile(llvm::LLVMContext& Context, const std::string& InputFilename);
    llvm::Function* findFunction(llvm::Module& M, const std::string& FName);
}  // namespace llvm_util

#endif
