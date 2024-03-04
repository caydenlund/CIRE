// Project files
#include "utils.h"
#include "Cire.h"

#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Passes/PassBuilder.h"


using namespace std;
using namespace llvm;
using namespace llvm_util;

namespace {
    cl::opt<string> File(cl::desc("Name of the file containing the bitcode"),
                             cl::value_desc("Gets the LLVM Module from this file"),
                             cl::Positional,
                             cl::Required);

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


}


int main(int argc, char **argv) {
  if (true) {
    // FIXME remove when done debugging
    for (int i = 0; i < argc; ++i)
      cout << "'" << argv[i] << "' ";
    cout << endl;
  }

  sys::PrintStackTraceOnErrorSignal(argv[0]);
  llvm::EnableDebugBuffering = true;
  llvm::llvm_shutdown_obj llvm_shutdown; // Call llvm_shutdown() on exit.
  llvm::LLVMContext Context;

  string Usage = "CIRE stand-alone worst-case floating-point round-off error estimator\n";
  cl::ParseCommandLineOptions(argc, argv, Usage);

  auto M = openInputFile(Context, File);
  if (!M) {
    errs() << "Error: Unable to open input file\n";
    return 1;
  }

  Cire cire;
  if(!Func.empty()) {
    auto F = findFunction(*M, Func);
    if (!F) {
      errs() << "Error: Unable to find function " << Func << " in the input file\n";
      return 1;
    }
    cire.graph->parse(*Input.c_str());


  }


//  cire.setFile(Input);
//  cire.setAbstaction(true);
//  cire.setAbstractionWindow(std::make_pair(10, 40));
//  cire.setMinDepth(1);
//  cire.setMaxDepth(10);
//  cire.performErrorAnalysis();
  return 0;
}