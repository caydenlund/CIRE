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

    cl::opt<unsigned> DebugLevel("debug-level",
                             cl::desc("Set the debug level"),
                             cl::init(0));

    cl::opt<unsigned> LogLevel("log-level",
                            cl::desc("Set the log level"),
                            cl::init(0));
    
    cl::opt<string> LogOutput("log-output",
                              cl::desc("Set the log output file"),
                              cl::value_desc("Sets the log output file"),
                              cl::init("default.log"));

    cl::opt<bool> CSVFriendly("csv-friendly",
                        cl::desc("Enable output to JSON file in a CSV friendly manner"),
                        cl::init(false));

    cl::opt<string> Output("output",
                           cl::desc("Set the output file"),
                           cl::value_desc("Sets the output file for the results"),
                           cl::init("results.json"));

    cl::opt<unsigned> OptimizerTimeOut("global-opt-timeout",
                                       cl::desc("Timeout for the optimizer. Default: 20 seconds"),
                                       cl::init(UINT_MAX));

    cl::opt<unsigned int> OperatorThreshold("operator-threshold",
                                            cl::desc("Set the threshold on number of operators on the error expression"),
                                            cl::init(10000));

    cl::opt<bool> ConcretizeErrorComponents("concretize-error-components",
                                            cl::desc("Concretize error components"),
                                            cl::init(false));

    cl::opt<bool> CollectErrorComponentData("collect-error-component-data",
                                            cl::desc("Enable collection of backward derivatives and local errors"),
                                            cl::init(false));
}


int main(int argc, char **argv) {
  const auto start = std::chrono::high_resolution_clock::now();
  if (true) {
    for (int i = 0; i < argc; ++i)
      cout << "'" << argv[i] << "' ";
    cout << endl;
  }

  sys::PrintStackTraceOnErrorSignal(argv[0]);
  EnableDebugBuffering = true;
  LLVMContext Context;

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
  cire.setFile(File);

  if (Abstraction) {
    cire.setAbstraction(true);
    cire.setAbstractionWindow(std::make_pair(MinDepth.getValue(), MaxDepth.getValue()));
  }
  if (DebugLevel) {
    cire.setDebugLevel(DebugLevel);
    cire.graph->debugLevel = DebugLevel;
    cire.graph->errorAnalyzer->debugLevel = DebugLevel;
    cire.graph->ibexInterface->debugLevel = DebugLevel;
    cire.results->debugLevel = DebugLevel;
  }
  if (OptimizerTimeOut) {
    cire.graph->ibexInterface->optimizerTimeOut = OptimizerTimeOut;
  }
  if (OperatorThreshold) {
    cire.graph->errorAnalyzer->errorExpressionOperatorThreshold = OperatorThreshold;
  }
  if (LogLevel) {
    cire.setLogLevel(LogLevel);
    cire.graph->logLevel = LogLevel;
    cire.graph->errorAnalyzer->logLevel = LogLevel;
  }
  
  if (!Output.empty()) {
    cire.results->setFile(Output);
  }

  if(cire.debugLevel > 0) {
    std::cout << "Parsing LLVM IR..." << std::endl;
  }
  if (cire.logLevel > 0) {
    std::cout << "Parsing LLVM IR..." << std::endl;
  }
  
  if(!LogOutput.empty()) {
    cire.graph->log.setFile(LogOutput);
  }

  if (ConcretizeErrorComponents) {
    cire.graph->concretize_error_components = true;
  }
  
  if (CollectErrorComponentData) {
    cire.setCollectErrorComponentData(true);
  }
  
  cire.graph->log.openFile();

  if(!Func.empty()) {
    auto F = findFunction(*M, Func);
    if (!F) {
      errs() << "Error: Unable to find function " << Func << " in the input file\n";
      return 1;
    }

    if(!Input.empty()) {
      cire.graph->parse(*Input.c_str());
    } else {
      parseInputsInLLVM(*cire.graph, *F);
    }

    // map the LLVM function arguments to the CIRE inputs
    // Iterate the function arguments
    for (auto &arg : F->args()) {
      if (arg.getType()->isFloatingPointTy()) {
        llvmToCireNodeMap[&arg] = cire.graph->symbolTables[cire.graph->currentScope]->table[arg.getName().str()];
        cireToLLVMNodeMap[cire.graph->symbolTables[cire.graph->currentScope]->table[arg.getName().str()]] = &arg;
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
          llvmToCireNodeMap[&arg] = cire.graph->symbolTables[cire.graph->currentScope]->table[arg.getName().str()];
          cireToLLVMNodeMap[cire.graph->symbolTables[cire.graph->currentScope]->table[arg.getName().str()]] = &arg;
        }
      }

      parseExprsInLLVM(*cire.graph, srcFn);
      break;
    }
  }
  const auto parse_end = std::chrono::high_resolution_clock::now();

  if (cire.debugLevel > 0) {
    std::cout << "Parsing complete" << std::endl;
  }
  if (cire.logLevel > 0) {
    std::cout << "Parsing complete" << std::endl;
  }

  std::map<Node *, ErrorAnalysisResult> answer = cire.performErrorAnalysis();

  const auto error_analysis_end = std::chrono::high_resolution_clock::now();

  if(cire.debugLevel > 0) {
    // print the result of nodes corresponding nodes in the output list
    for (string &output: cire.graph->outputs) {
      Node *node = cire.graph->symbolTables[cire.graph->currentScope]->table[output];
      assert(answer.find(node) != answer.end());

      std::cout << "\nOutput Variable: " << output << "";
      if(cire.debugLevel > 2) {
        std::cout << *node;
      }
      std::cout << ": \tOutput: " << answer[node].outputExtrema << ","
                << "  \tError: " << answer[node].errorExtrema << ","
                << "  \n\tNum Optimizer Calls: " << answer[node].numOptimizationCalls << std::endl;
    }
  }

  if (cire.logLevel > 0) {
    // print the result of nodes corresponding nodes in the output list
    for (string &output: cire.graph->outputs) {
      Node *node = cire.graph->symbolTables[cire.graph->currentScope]->table[output];
      assert(answer.find(node) != answer.end());

      cire.graph->log.logFile << "\nOutput Variable: " << output << "";
      if(cire.debugLevel > 2) {
        cire.graph->log.logFile << *node;
      }
      cire.graph->log.logFile << ": \tOutput: " << answer[node].outputExtrema << ","
                              << "  \tError: " << answer[node].errorExtrema << ","
                              << "  \n\tNum Optimizer Calls: " << answer[node].numOptimizationCalls << std::endl;
    }
  }

  const auto end = std::chrono::high_resolution_clock::now();
  cire.time_map["Parsing"] = parse_end - start;
  cire.time_map["Error_Analysis"] = error_analysis_end - parse_end;
  cire.time_map["Total"] = end - start;

  if(cire.debugLevel > 0) {
    std::cout << "Parsing Time taken: " << cire.time_map["Parsing"].count() << " seconds" << std::endl;
    std::cout << "Error Analysis Time taken: " << cire.time_map["Error Analysis"].count() << " seconds" << std::endl;
    std::cout << "Total Time taken: " << cire.time_map["Total"].count() << " seconds" << std::endl;
  }
  if(cire.logLevel > 0) {
    cire.graph->log.logFile << "Parsing Time taken: " << cire.time_map["Parsing"].count() << " seconds" << std::endl;
    cire.graph->log.logFile << "Error Analysis Time taken: " << cire.time_map["Error Analysis"].count() << " seconds" << std::endl;
    cire.graph->log.logFile << "Total Time taken: " << cire.time_map["Total"].count() << " seconds" << std::endl;
    cire.graph->log.log("Writing results to " + cire.results->file + " ...");
  }


  if(CSVFriendly) {
    cire.results->writeResultsForCSV(cire.graph->outputs,
                                     cire.graph->numOperatorsOutput,
                                     cire.graph->depthTable.rbegin()->first,
                                     cire.graph->abstractionMetrics,
                                     cire.file, answer, cire.time_map);
  } else {
    cire.results->writeResults(cire.graph->outputs,
                               cire.graph->numOperatorsOutput,
                               cire.graph->depthTable.rbegin()->first,
                               cire.graph->abstractionMetrics,
                               cire.file, answer, cire.time_map);
  }



  if(cire.logLevel > 0) {
    cire.graph->log.log("Results written to " + cire.results->file + "!");
  }

  return 0;
}
