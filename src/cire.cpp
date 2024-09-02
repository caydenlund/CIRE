//=============================================================================
// FILE:
//    CIRE_LLVM.cpp
//
// DESCRIPTION:
//    Visits all functions in a module, prints their names and the number of
//    arguments via stderr. Strictly speaking, this is an analysis pass (i.e.
//    the functions are not modified). However, in order to keep things simple
//    there's no 'print' method here (every analysis pass should implement it).
//
// USAGE:
//    New PM
//      opt -load-pass-plugin=libCIRE_LLVM.dylib -passes="cire" `\`
//        -disable-output <input-llvm-file>
//
//
// License: MIT
//=============================================================================
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "Cire.h"
#include "llvm_frontend.h"

using namespace llvm;

// No need to expose the internals of the pass to the outside world - keep
// everything in an anonymous namespace.
namespace {
    cl::opt<string> Func("function",
                          cl::desc("Function to analyze"),
                          cl::value_desc("Used for matching the function name"),
                          cl::init(""),
                          cl::Required);

    cl::opt<string> Input("input",
                          cl::desc("Input file for CIRE containing INPUTS and OUTPUTS"),
                          cl::value_desc("Looks for the input file in the current directory"),
                          cl::init(""),
                          cl::Required);

// This method implements what the pass does
    void visitor(Function &F) {
      errs() << "(llvm-tutor) Hello from: "<< F.getName() << "\n";
      errs() << "(llvm-tutor)   number of arguments: " << F.arg_size() << "\n";
    }

// New PM implementation
    struct CIRE_LLVM : PassInfoMixin<CIRE_LLVM> {
        // Main entry point, takes IR unit to run the pass on (&F) and the
        // corresponding pass manager (to be queried if need be)
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
            visitor(F);

            // Identify the function
            if (F.getName() == Func) {
              Cire cire;

              // Parse the input file
              cire.graph->parse(*Input.c_str());

              // Parse the function
              parseExprsInLLVM(*cire.graph, F);
            }



            return PreservedAnalyses::all();
        }

        // Without isRequired returning true, this pass will be skipped for functions
        // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
        // all functions with optnone.
        static bool isRequired() { return true; }
    };

} // namespace

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getCIREPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "CIRE", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
              PB.registerPipelineParsingCallback(
                      [](StringRef Name, FunctionPassManager &FPM,
                         ArrayRef<PassBuilder::PipelineElement>) {
                          if (Name == "cire") {
                            FPM.addPass(CIRE_LLVM());
                            return true;
                          }
                          return false;
                      });
          }};
}

// This is the core interface for pass plugins. It guarantees that 'opt' will
// be able to recognize CIRE_LLVM when added to the pass pipeline on the
// command line, i.e. via '-passes=cire'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getCIREPluginInfo();
}