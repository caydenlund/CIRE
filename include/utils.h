#include <string>
#include <memory>

namespace llvm {
  class Module;
  class LLVMContext;
  class Function;
}

namespace IR {
  class Function;
}

namespace llvm_util {
    std::unique_ptr<llvm::Module> openInputFile(llvm::LLVMContext &Context,
                                                const std::string &InputFilename);
    llvm::Function *findFunction(llvm::Module &M, const std::string &FName);
}