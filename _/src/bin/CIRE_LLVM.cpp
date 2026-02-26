#include <fstream>
#include <memory>
#include <sstream>

#include "cire/core/Cire.h"
#include "cire/core/Node.hpp"
#include "cire/frontend/llvm_frontend.h"
#include "cire/interfaces/Logging.h"
#include "cire/utils/utils.h"

#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"

using namespace std;
using namespace llvm;
using namespace llvm_util;

struct CliOpts {
    cl::opt<string> file {cl::desc("Name of the file containing the bitcode"),
                          cl::value_desc("Gets the LLVM Module from this file"), cl::Positional, cl::Required};

    cl::opt<string> func {"function", cl::desc("Function to analyze"),
                          cl::value_desc("Used for matching the function name"), cl::init("")};

    cl::opt<string> input {"input", cl::desc("Input file for CIRE containing INPUTS and OUTPUTS"),
                           cl::value_desc("Looks for the input file in the current directory"), cl::init("")};

    cl::opt<bool> abstraction {"abstraction", cl::desc("Enable abstraction"), cl::init(false)};

    cl::opt<unsigned> minDepth {"min-depth", cl::desc("Minimum depth for abstraction"), cl::init(5)};

    cl::opt<unsigned> maxDepth {"max-depth", cl::desc("Maximum depth for abstraction"), cl::init(10)};

    cl::opt<unsigned> logLevel {"log-level", cl::desc("Set the log level"), cl::init(2)};

    cl::opt<string> logOutput {"log-output", cl::desc("Set the log output file"),
                               cl::value_desc("Sets the log output file"), cl::init("")};

    cl::opt<bool> csvFriendly {"csv-friendly", cl::desc("Enable output to JSON file in a CSV friendly manner"),
                               cl::init(false)};

    cl::opt<string> output {"output", cl::desc("Set the output file"),
                            cl::value_desc("Sets the output file for the results"), cl::init("results.json")};

    cl::opt<unsigned> optimizerTimeout {"global-opt-timeout",
                                        cl::desc("Timeout for the optimizer. Default: 20 seconds"), cl::init(UINT_MAX)};

    cl::opt<unsigned int> operatorThreshold {
            "operator-threshold", cl::desc("Set the threshold on number of operators on the error expression"),
            cl::init(10000)};

    cl::opt<bool> concretizeErrorComponents {"concretize-error-components", cl::desc("Concretize error components"),
                                             cl::init(false)};

    cl::opt<bool> collectErrorComponentData {"collect-error-component-data",
                                             cl::desc("Enable collection of backward derivatives and local errors"),
                                             cl::init(false)};

    cl::opt<bool> stdoutOutput {"stdout", cl::desc("Print output to stdout in non-nested format for compiler explorer"),
                                cl::init(false)};

    cl::opt<bool> showAllInstructions {"show-all-instructions",
                                       cl::desc("Show all instructions including those with zero error contribution"),
                                       cl::init(false)};

    cl::opt<string> inputBounds {
            "input-bounds", cl::desc("Specify input bounds as param:min:max,param2:min:max (default: -1:1 for all)"),
            cl::value_desc("Comma-separated list of parameter bounds"), cl::init("")};
};


std::map<std::string, std::pair<double, double>> parseInputBounds(const std::string& boundsStr) {
    std::map<std::string, std::pair<double, double>> boundsMap;

    if (boundsStr.empty()) { return boundsMap; }

    std::stringstream ss(boundsStr);
    std::string token;

    while (std::getline(ss, token, ',')) {
        std::stringstream tokenStream(token);
        std::string param;
        std::string minStr;
        std::string maxStr;

        if (std::getline(tokenStream, param, ':') && std::getline(tokenStream, minStr, ':')
            && std::getline(tokenStream, maxStr)) {
            try {
                double min = std::stod(minStr);
                double max = std::stod(maxStr);
                boundsMap[param] = {min, max};
            } catch (const std::exception& e) {
                errs() << "Error parsing bounds for parameter '" << param << "': " << e.what() << "\n";
            }
        } else {
            errs() << "Invalid bounds format for token '" << token << "'. Expected format: param:min:max\n";
        }
    }

    return boundsMap;
}


int main(int argc, char** argv) {
    CliOpts opts {};

    string usage = "CIRE stand-alone worst-case floating-point round-off error estimator\n";
    cl::ParseCommandLineOptions(argc, argv, usage);

    logging = std::make_unique<Logging>(std::cout, LogLevel(opts.logLevel.getValue()));
    const auto start = std::chrono::high_resolution_clock::now();

    sys::PrintStackTraceOnErrorSignal(argv[0]);
    EnableDebugBuffering = true;
    LLVMContext context;

    auto llvmModule = openInputFile(context, opts.file);
    if (!llvmModule) {
        errs() << "Error: Unable to open input file\n";
        return 1;
    }

    // TODO: Think about what programs you cannot accept and handle them.
    //  Programs with no return values - Let through. Cant analyze
    //  Programs with no inputs - Let through. Cant analyze
    //  Programs with no floating-point inputs OR return values - Let through. Cant analyze
    //  Programs with vectors as inputs or outputs can be left for later - Cant analyze

    Cire cire;
    cire.setFile(opts.file.getValue());

    if (opts.abstraction) {
        cire.setAbstraction(true);
        cire.setAbstractionWindow(std::make_pair(opts.minDepth.getValue(), opts.maxDepth.getValue()));
    }

    if (opts.optimizerTimeout > 0) cire.graph->ibexInterface->optimizerTimeOut = opts.optimizerTimeout;
    if (opts.operatorThreshold > 0)
        cire.graph->errorAnalyzer->errorExpressionOperatorThreshold = opts.operatorThreshold;

    if (!opts.output.empty()) cire.results->setFile(opts.output.getValue());

    logging->debug("Parsing LLVM IR...");

    std::unique_ptr<std::ofstream> logFile;
    if (!opts.logOutput.empty()) {
        logFile = std::make_unique<std::ofstream>(opts.logOutput.getValue());
        logging = std::make_unique<Logging>(*logFile, LogLevel(opts.logLevel.getValue()));
    }

    if (opts.concretizeErrorComponents) cire.graph->concretizeErrorComps = true;

    if (opts.collectErrorComponentData) cire.setCollectErrorComponentData(true);

    if (opts.stdoutOutput) cire.results->setStdoutOutput(true);

    if (opts.showAllInstructions) cire.results->setShowAllInstructions(true);

    // Parse input bounds from command-line argument
    auto inputBoundsMap = parseInputBounds(opts.inputBounds.getValue());

    if (!opts.func.empty()) {
        auto* func = findFunction(*llvmModule, opts.func);
        if (func == nullptr) {
            logging->critical("Unable to find function '" + opts.func.getValue() + "' in the input file");
        }

        if (!opts.input.empty()) {
            cire.graph->parse(*opts.input.c_str());
        } else {
            parseInputsInLLVM(*cire.graph, *func, inputBoundsMap);
        }

        // Note: LLVM function arguments are now automatically mapped in parseInputsInLLVM
        parseExprsInLLVM(*cire.graph, *func);
    } else {
        for (auto& func : *llvmModule) {
            if (func.isDeclaration()) continue;

            if (!opts.input.empty()) {
                cire.graph->parse(*opts.input.c_str());
            } else {
                parseInputsInLLVM(*cire.graph, func, inputBoundsMap);
            }

            // Note: LLVM function arguments are now automatically mapped in parseInputsInLLVM
            parseExprsInLLVM(*cire.graph, func);
            break;
        }
    }

    const auto parseEnd = std::chrono::high_resolution_clock::now();

    logging->debug("Parsing complete");
    logging->info("Performing error analysis");

    std::map<ir::Node*, ErrorAnalysisResult> answer = cire.performErrorAnalysis();

    const auto errorAnalysisEnd = std::chrono::high_resolution_clock::now();

    // print the result of nodes corresponding nodes in the output list
    for (string& output : cire.graph->outputs) {
        ir::Node* node = cire.graph->symbolTables[cire.graph->currentScope]->table[output];
        assert(answer.find(node) != answer.end());

        logging->debug("Output variable: '" + output + "'");

        std::stringstream outputExtrema;
        outputExtrema << answer[node].outputExtrema;
        logging->debug("    Output: " + outputExtrema.str());

        std::stringstream errorExtrema;
        errorExtrema << answer[node].errorExtrema;
        logging->debug("    Error:  " + errorExtrema.str());

        std::stringstream numOptCalls;
        numOptCalls << answer[node].numOptimizationCalls;
        logging->debug("    Optimizer Call Count:  " + numOptCalls.str());
    }

    const auto end = std::chrono::high_resolution_clock::now();
    cire.timeMap["Parsing"] = parseEnd - start;
    cire.timeMap["Error_Analysis"] = errorAnalysisEnd - parseEnd;
    cire.timeMap["Total"] = end - start;

    logging->info("Parsing Time taken: " + std::to_string(cire.timeMap["Parsing"].count()) + " seconds");
    logging->info("Error Analysis Time taken: " + std::to_string(cire.timeMap["Error Analysis"].count()) + " seconds");
    logging->info("Total Time taken: " + std::to_string(cire.timeMap["Total"].count()) + " seconds");


    auto instructionErrors = cire.graph->errorAnalyzer->getInstructionErrorBreakdown(cire.graph->ibexInterface,
                                                                                     cire.graph);

    if (opts.csvFriendly) {
        cire.results->writeResultsForCSV(cire.graph->outputs, cire.graph->numOperatorsOutput,
                                         cire.graph->depthTable.rbegin()->first, cire.graph->abstractionMetrics,
                                         cire.file, answer, cire.timeMap, instructionErrors);
    } else {
        cire.results->writeResults(cire.graph->outputs, cire.graph->numOperatorsOutput,
                                   cire.graph->depthTable.rbegin()->first, cire.graph->abstractionMetrics, cire.file,
                                   answer, cire.timeMap, instructionErrors);
    }

    logging->info("Results written to '" + cire.results->file + "'");

    return 0;
}
