// Project files
#include "utils.h"
#include "Cire.h"
#include "../util/llvm_frontend.cpp"

#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/Module.h"

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
                         cl::init(""));

    cl::opt<string> Input("input",
                          cl::desc("Input file for CIRE containing INPUTS and OUTPUTS"),
                          cl::value_desc("Looks for the input file in the current directory"),
                          cl::init(""));

    cl::opt<bool> Abstraction("abstraction",
                              cl::desc("Enable abstraction"),
                              cl::init(false));

    cl::opt<unsigned> MinDepth("min-depth",
                               cl::desc("Minimum depth for abstraction"),
                               cl::init(5));

    cl::opt<unsigned> MaxDepth("max-depth",
                                cl::desc("Maximum depth for abstraction"),
                                cl::init(10));



}


int main(int argc, char **argv) {
  const auto start = std::chrono::high_resolution_clock::now();
  if (true) {
    // FIXME remove when done debugging
    for (int i = 0; i < argc; ++i)
      cout << "'" << argv[i] << "' ";
    cout << endl;
  }

  sys::PrintStackTraceOnErrorSignal(argv[0]);
  llvm::EnableDebugBuffering = true;
  llvm::LLVMContext Context;

  string Usage = "CIRE stand-alone worst-case floating-point round-off error estimator\n";
  cl::ParseCommandLineOptions(argc, argv, Usage);

  auto M = openInputFile(Context, File);
  if (!M) {
    errs() << "Error: Unable to open input file\n";
    return 1;
  }

  // TODO: Think about what programs you cannot accept and handle them.
  //  Programs with no return values - Let through. Cant analyze
  //  Programs with no inputs - Let through. Cant analyze
  //  Programs with no floating-point inputs OR return values - Let through. Cant analyze
  //  Programs with vectors as inputs or outputs can be left for later - Cant analyze

  Cire cire;
  if(!Func.empty()) {
    auto F = findFunction(*M, Func);
    if (!F) {
      errs() << "Error: Unable to find function " << Func << " in the input file\n";
      return 1;
    }

    if(!Input.empty()) {
      cire.graph->parse(*Input.c_str());
    }
    else {
      parseInputsInLLVM(*cire.graph, *F);
    }

    // map the LLVM function arguments to the CIRE inputs
    // Iterate the function arguments
    for (auto &arg : F->args()) {
      if (arg.getType()->isFloatingPointTy()) {
        llvmToCireNodeMap[&arg] = cire.graph->symbolTables[cire.graph->currentScope]->table[arg.getNameOrAsOperand()];
        cireToLLVMNodeMap[cire.graph->symbolTables[cire.graph->currentScope]->table[arg.getNameOrAsOperand()]] = &arg;
      }
    }

    parseExprsInLLVM(*cire.graph, *F);
  } else {
    for (auto &srcFn : *M) {
      if (srcFn.isDeclaration()) {
        continue;
      }

      if(!Input.empty()) {
        cire.graph->parse(*Input.c_str());
      }
      else {
        parseInputsInLLVM(*cire.graph, srcFn);
      }

      // map the LLVM function arguments to the CIRE inputs
      // Iterate the function arguments
      for (auto &arg : srcFn.args()) {
        if (arg.getType()->isFloatingPointTy()) {
          llvmToCireNodeMap[&arg] = cire.graph->symbolTables[cire.graph->currentScope]->table[arg.getNameOrAsOperand()];
          cireToLLVMNodeMap[cire.graph->symbolTables[cire.graph->currentScope]->table[arg.getNameOrAsOperand()]] = &arg;
        }
      }

      parseExprsInLLVM(*cire.graph, srcFn);
      break;
    }
  }
  const auto parse_end = std::chrono::high_resolution_clock::now();

  std::cout << "Parsing complete" << std::endl;

  std::map<Node *, std::vector<ibex::Interval>> answer = cire.performErrorAnalysis();

  const auto error_analysis_end = std::chrono::high_resolution_clock::now();

  unsigned i = 0;
  // print the answer map
  for (auto const &[node, result]: answer) {
    // This assumes that the map nodes are ordered in the same way as the outputs list nodes which is not
    // always the case
    assert(cire.graph->symbolTables[i]->table[cire.graph->outputs[i]] == node);
    std::cout << *node << " : " << "\n\tOutput: " << result[0] << ","
              << "\n\tError: " << result[1] << std::endl;
  }

  const auto end = std::chrono::high_resolution_clock::now();
  cire.time_map["Parsing"] = parse_end - start;
  cire.time_map["Error_Analysis"] = error_analysis_end - parse_end;
  cire.time_map["Total"] = end - start;

  std::cout << "Parsing Time taken: " << cire.time_map["Parsing"].count() << " seconds" << std::endl;
  std::cout << "Error Analysis Time taken: " << cire.time_map["Error Analysis"].count() << " seconds" << std::endl;
  std::cout << "Total Time taken: " << cire.time_map["Total"].count() << " seconds" << std::endl;

//  cire.results->writeResults(cire.graph->outputs, answer, cire.time_map);
  return 0;
}
