#include "cire/utils/utils.h"
#include "llvm/Support/Error.h"

#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

namespace llvm_util {
    std::unique_ptr<llvm::Module> openInputFile(llvm::LLVMContext& Context, const std::string& InputFilename) {
        llvm::ExitOnError exitOnErr;
        auto MB = exitOnErr(errorOrToExpected(llvm::MemoryBuffer::getFile(InputFilename)));
        llvm::SMDiagnostic Diag;
        auto M = getLazyIRModule(std::move(MB), Diag, Context, true);

        if (!M) {
            Diag.print("llvm_util", llvm::errs(), false);
            return {};
        }
        exitOnErr(M->materializeAll());
        return M;
    }

    llvm::Function* findFunction(llvm::Module& M, const std::string& FName) {
        auto* F = M.getFunction(FName);
        return (F != nullptr) && !F->isDeclaration() ? F : nullptr;
    }
}  // namespace llvm_util
