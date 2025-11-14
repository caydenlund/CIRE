#include "cire/core/Cire.h"
#include "cire/interfaces/Logging.h"

void show_usage(std::string name) {
    if (logging) {
        logging->info("Usage: ", name, " <option(s)> <input_file>");
        logging->info("Options:");
        logging->info("\t-h,--help\t\tShow this help message");
        logging->info("\t-a,--abstraction\t\tEnable abstraction");
        logging->info("\t-m,--min-depth\t\tSet minimum depth for abstraction");
        logging->info("\t-M,--max-depth\t\tSet maximum depth for abstraction");
        logging->info("\t-c,--compare\t\tValidate the generated error expression with the one in the file");
        logging->info("\t-o,--output\t\tSet the output file");
        logging->info("\t-d,--debug-level\t\tSet the debug level");
        logging->info("\t-l,--log-level\t\tSet the log level");
        logging->info("\t-lo,--log-output\t\tSet the log output file");
        logging->info("\t-cf,--csv-friendly\t\tEnable output to JSON file in a CSV friendly manner");
        logging->info("\t-to,--global-opt-timeout\t\tSet the global optimization timeout");
        logging->info("\t-op,--operator-threshold\t\tSet the threshold on numer of operators on the error expression");
        logging->info("\t-cec,--concretize-error-components\t\tConcretize error components");
        logging->info("\t-cecd,--collect-error-component-data\t\tCollect error component data");
        logging->info("\t--stdout\t\tPrint output to stdout in non-nested format for compiler explorer");
    } else {
        std::cerr << "Usage: " << name << " <option(s)> <input_file>"
                  << "Options:\n"
                  << "\t-h,--help\t\tShow this help message\n"
                  << "\t-a,--abstraction\t\tEnable abstraction\n"
                  << "\t-m,--min-depth\t\tSet minimum depth for abstraction\n"
                  << "\t-M,--max-depth\t\tSet maximum depth for abstraction\n"
                  << "\t-c,--compare\t\tValidate the generated error expression with the one in the "
                     "file\n"
                  << "\t-o,--output\t\tSet the output file\n"
                  << "\t-d,--debug-level\t\tSet the debug level\n"
                  << "\t-l,--log-level\t\tSet the log level\n"
                  << "\t-lo,--log-output\t\tSet the log output file\n"
                  << "\t-cf,--csv-friendly\t\tEnable output to JSON file in a CSV friendly manner\n"
                  << "\t-to,--global-opt-timeout\t\tSet the global optimization timeout\n"
                  << "\t-op,--operator-threshold\t\tSet the threshold on numer of operators on the "
                     "error expression\n"
                  << "\t-cec,--concretize-error-components\t\tConcretize error components\n"
                  << "\t-cecd,--collect-error-component-data\t\tCollect error component data\n"
                  << "\t--stdout\t\tPrint output to stdout in non-nested format for compiler explorer\n"
                  << "\n";
    }
}

int main(int argc, char* argv[]) {
    // Initialize logging system
    logging = std::make_unique<Logging>(std::cout, LogLevel::WARN);
    
    Cire cire;

    const auto start = std::chrono::high_resolution_clock::now();
    bool CSV_friendly = false;

    if (argc < 2) {
        show_usage(argv[0]);
        return 1;
    }
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return 0;
        }
        if ((arg == "-a") || (arg == "--abstraction")) {
            cire.setAbstraction(true);
        } else if ((arg == "-m") || (arg == "--min-depth")) {
            if (i + 1 < argc) {
                cire.setMinDepth(std::stoi(argv[++i]));
            } else {
                if (logging) {
                    logging->error("--min-depth option requires one argument.");
                } else {
                    std::cerr << "--min-depth option requires one argument.\n";
                }
                return 1;
            }
        } else if ((arg == "-M") || (arg == "--max-depth")) {
            if (i + 1 < argc) {
                cire.setMaxDepth(std::stoi(argv[++i]));
            } else {
                if (logging) {
                    logging->error("--max-depth option requires one argument.");
                } else {
                    std::cerr << "--max-depth option requires one argument.\n";
                }
                return 1;
            }
        } else if ((arg == "-c") || (arg == "--compare")) {
            if (i + 1 < argc) {
                cire.graph->setValidationFile(argv[++i]);
            } else {
                if (logging) {
                    logging->error("--compare option requires one argument.");
                } else {
                    std::cerr << "--compare option requires one argument.\n";
                }
                return 1;
            }
        } else if ((arg == "-o" || (arg == "--output"))) {
            if (i + 1 < argc) {
                cire.results->setFile(argv[++i]);
            } else {
                if (logging) {
                    logging->error("--output option requires one argument.");
                } else {
                    std::cerr << "--output option requires one argument.\n";
                }
                return 1;
            }
        } else if ((arg == "-d" || (arg == "--debug-level"))) {
            if (++i < argc) {
                // TODO
            } else {
                if (logging) {
                    logging->error("--debug-level option requires one argument.");
                } else {
                    std::cerr << "--debug-level option requires one argument.\n";
                }
                return 1;
            }
        } else if ((arg == "-l" || (arg == "--log-level"))) {
            if (++i < argc) {
                // TODO
            } else {
                if (logging) {
                    logging->error("--log-level option requires one argument.");
                } else {
                    std::cerr << "--log-level option requires one argument.\n";
                }
                return 1;
            }
        } else if ((arg == "-lo") || (arg == "--log-output")) {
            if (++i < argc) {
                // TODO
            } else {
                if (logging) {
                    logging->error("--log-output option requires one argument.");
                } else {
                    std::cerr << "--log-output option requires one argument.\n";
                }
                return 1;
            }
        } else if ((arg == "-cf" || (arg == "--csv-friendly"))) {
            CSV_friendly = true;
        } else if ((arg == "-to" || (arg == "--global-opt-timeout"))) {
            if (i + 1 < argc) {
                cire.graph->ibexInterface->optimizerTimeOut = std::stoi(argv[++i]);
            } else {
                if (logging) {
                    logging->error("--global-opt-timeout option requires one argument. Default: 20 seconds");
                } else {
                    std::cerr << "--global-opt-timeout option requires one argument. Default: 20 "
                                 "seconds"
                              << "\n";
                }
                return 1;
            }
        } else if ((arg == "-op" || (arg == "--operator-threshold"))) {
            if (i + 1 < argc) {
                cire.graph->errorAnalyzer->errorExpressionOperatorThreshold = std::stoi(argv[++i]);
            } else {
                if (logging) {
                    logging->error("--operator-threshold option requires one argument.");
                } else {
                    std::cerr << "--operator-threshold option requires one argument.\n";
                }
                return 1;
            }
        } else if ((arg == "-cec") || (arg == "--concretize-error-components")) {
            cire.graph->concretizeErrorComps = true;
        } else if ((arg == "-cecd") || (arg == "--collect-error-component-data")) {
            cire.setCollectErrorComponentData(true);
        } else if ((arg == "--stdout")) {
            cire.results->setStdoutOutput(true);
        } else {
            cire.setFile(argv[i]);
        }
    }


    if (cire.graph->parse(*cire.file.c_str()) != 0) { return 1; }

    const auto parse_end = std::chrono::high_resolution_clock::now();

    std::map<Node*, ErrorAnalysisResult> answer = cire.performErrorAnalysis();

    const auto error_analysis_end = std::chrono::high_resolution_clock::now();

    if (logging && logging->level <= LogLevel::INFO) {
        // print the result of nodes corresponding nodes in the output list
        for (string& output : cire.graph->outputs) {
            Node* node = cire.graph->symbolTables[cire.graph->currentScope]->table[output];
            assert(answer.find(node) != answer.end());

            logging->info("\nOutput Variable: ", output);
            if (logging->level <= LogLevel::DEBUG) {
                std::cout << *node;
            }
            logging->info(": \tOutput: ", answer[node].outputExtrema.lb(), ", ", answer[node].outputExtrema.ub());
            logging->info("  \tError: ", answer[node].errorExtrema.lb(), ", ", answer[node].errorExtrema.ub());
            logging->info("  \n\tNum Optimizer Calls: ", answer[node].numOptimizationCalls);
        }
    }


    const auto total_end = std::chrono::high_resolution_clock::now();
    cire.timeMap["Parsing"] = parse_end - start;
    cire.timeMap["Error_Analysis"] = error_analysis_end - parse_end;
    cire.timeMap["Total"] = total_end - start;

    if (logging) {
        logging->info("Parsing Time taken: ", cire.timeMap["Parsing"].count(), " seconds");
        logging->info("Error Analysis Time taken: ", cire.timeMap["Error_Analysis"].count(), " seconds");
        logging->info("Time taken: ", cire.timeMap["Total"].count(), " seconds");
        logging->debug("Writing results to ", cire.results->file, " ...");
    }

    auto instructionErrors = cire.graph->errorAnalyzer->getInstructionErrorBreakdown(cire.graph->ibexInterface, cire.graph);
    
    if (CSV_friendly) {
        cire.results->writeResultsForCSV(cire.graph->outputs, cire.graph->numOperatorsOutput,
                                         cire.graph->depthTable.rbegin()->first, cire.graph->abstractionMetrics,
                                         cire.file, answer, cire.timeMap, instructionErrors);
    } else {
        cire.results->writeResults(cire.graph->outputs, cire.graph->numOperatorsOutput,
                                   cire.graph->depthTable.rbegin()->first, cire.graph->abstractionMetrics, cire.file,
                                   answer, cire.timeMap, instructionErrors);
    }

    if (logging) {
        logging->debug("Results written to ", cire.results->file, "!");
    }

    return 0;
}
