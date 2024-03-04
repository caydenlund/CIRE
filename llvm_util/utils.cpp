// Project Files
#include "utils.h"

#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/Module.h>
#include "llvm/Support/Error.h"
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

using namespace std;
using namespace llvm;

static llvm::ExitOnError ExitOnErr;

namespace llvm_util {
    std::unique_ptr<llvm::Module> openInputFile(llvm::LLVMContext &Context,
                                                const string &InputFilename) {
      auto MB = ExitOnErr(errorOrToExpected(MemoryBuffer::getFile(InputFilename)));
      llvm::SMDiagnostic Diag;
      auto M = getLazyIRModule(std::move(MB), Diag, Context, true);

      if(!M) {
        Diag.print("llvm_util", llvm::errs(), false);
        return 0;
      }
      ExitOnErr(M->materializeAll());
      return M;
    }

    Function *findFunction(Module &M, const string &FName) {
      auto F = M.getFunction(FName);
      return F && !F->isDeclaration()?F: nullptr;
    }
}