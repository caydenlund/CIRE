#include "Cire.h"

void show_usage(std::string name) {
  std::cerr << "Usage: " << name << " <option(s)> <input_file>"
            << "Options:\n"
            << "\t-h,--help\t\tShow this help message\n"
            << "\t-a,--abstraction\t\tEnable abstraction\n"
            << "\t-m,--min-depth\t\tSet minimum depth for abstraction\n"
            << "\t-M,--max-depth\t\tSet maximum depth for abstraction\n"
            << "\t-c,--compare\t\tValidate the generated error expression with the one in the file\n"
            << "\t-o,--output\t\tSet the output file\n"
            << "\t-d,--debug-level\t\tSet the debug level\n"
            << "\t-l,--log-level\t\tSet the log level\n"
            << std::endl;
}

int main(int argc, char *argv[]) {
  Cire cire;

  const auto start = std::chrono::high_resolution_clock::now();

  if (argc < 2) {
    show_usage(argv[0]);
    return 1;
  } else {
    for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if ((arg == "-h") || (arg == "--help")) {
        show_usage(argv[0]);
        return 0;
      } else if ((arg == "-a") || (arg == "--abstraction")) {
        if (i + 1 < argc) {
          cire.setAbstaction(true);
        } else {
          std::cerr << "--abstraction option requires one argument." << std::endl;
          return 1;
        }
      } else if ((arg == "-m") || (arg == "--min-depth")) {
        if (i + 1 < argc) {
          cire.setMinDepth(std::stoi(argv[++i]));
        } else {
          std::cerr << "--min-depth option requires one argument." << std::endl;
          return 1;
        }
      } else if ((arg == "-M") || (arg == "--max-depth")) {
        if (i + 1 < argc) {
          cire.setMaxDepth(std::stoi(argv[++i]));
        } else {
          std::cerr << "--max-depth option requires one argument." << std::endl;
          return 1;
        }
      } else if ((arg == "-c") || (arg == "--compare")) {
        if (i + 1 < argc) {
          cire.graph->setValidationFile(argv[++i]);
        } else {
          std::cerr << "--compare option requires one argument." << std::endl;
          return 1;
        }
      } else if ((arg == "-o" || (arg == "--output"))) {
        if (i + 1 < argc) {
          cire.results->setFile(argv[++i]);
        } else {
          std::cerr << "--output option requires one argument." << std::endl;
          return 1;
        }
      } else if ((arg == "-d" || (arg == "--debug-level"))) {
        if (i + 1 < argc) {
          cire.setDebugLevel(std::stoi(argv[++i]));
          cire.graph->debugLevel = cire.debugLevel;
          cire.graph->errorAnalyzer->debugLevel = cire.debugLevel;
          cire.results->debugLevel = cire.debugLevel;
        } else {
          std::cerr << "--debug-level option requires one argument." << std::endl;
          return 1;
        }
      } else if ((arg == "-l" || (arg == "--log-level"))) {
        if (i + 1 < argc) {
          cire.setLogLevel(std::stoi(argv[++i]));
          cire.graph->logLevel = cire.logLevel;
          cire.graph->errorAnalyzer->logLevel = cire.logLevel;
          cire.results->logLevel = cire.logLevel;
        } else {
          std::cerr << "--log-level option requires one argument." << std::endl;
          return 1;
        }
      } else {
        cire.setFile(argv[i]);
      }
    }
  }

  if(cire.logLevel > 0) {
    cire.graph->log.openFile();
  }

  cire.graph->parse(*cire.file.c_str());

  const auto parse_end = std::chrono::high_resolution_clock::now();

  std::map<Node *, std::vector<ibex::Interval>> answer = cire.performErrorAnalysis();

  const auto error_analysis_end = std::chrono::high_resolution_clock::now();

  unsigned i = 0;

  if(cire.debugLevel > 0) {
    // print the answer map
    for (auto const &[node, result]: answer) {
      // This assumes that the map nodes are ordered in the same way as the outputs list nodes which is not
      // always the case
      assert(cire.graph->symbolTables[i]->table[cire.graph->outputs[i]] == node);
      std::cout
//        << *node << " : "
        << "\n\tOutput: " << result[0] << ","
        << "\n\tError: " << result[1] << std::endl;
    }
  }

  if (cire.logLevel > 0) {
    // print the answer map
    for (auto const &[node, result]: answer) {
      // This assumes that the map nodes are ordered in the same way as the outputs list nodes which is not
      // always the case
      assert(cire.graph->symbolTables[i]->table[cire.graph->outputs[i]] == node);
      cire.log.logFile
//        << *node << " : "
              << "\n\tOutput: " << result[0] << ","
              << "\n\tError: " << result[1] << std::endl;
    }
  }

  const auto total_end = std::chrono::high_resolution_clock::now();
  cire.time_map["Parsing"] = parse_end - start;
  cire.time_map["Error_Analysis"] = error_analysis_end - parse_end;
  cire.time_map["Total"] = total_end - start;

  if(cire.debugLevel > 0) {
    std::cout << "Parsing Time taken: " << cire.time_map["Parsing"].count() << " seconds" << std::endl;
    std::cout << "Error Analysis Time taken: " << cire.time_map["Error Analysis"].count() << " seconds" << std::endl;
    std::cout << "Time taken: " << cire.time_map["Total"].count() << " seconds" << std::endl;
  }
  if(cire.logLevel > 0) {
    cire.log.logFile << "Parsing Time taken: " << cire.time_map["Parsing"].count() << " seconds" << std::endl;
    cire.log.logFile << "Error Analysis Time taken: " << cire.time_map["Error Analysis"].count() << " seconds" << std::endl;
    cire.log.logFile << "Time taken: " << cire.time_map["Total"].count() << " seconds" << std::endl;
    cire.log.log("Writing results to " + cire.results->file + " ...\n");
  }


  cire.results->writeResults(cire.graph->outputs, answer, cire.time_map);

  if(cire.logLevel > 0) {
    cire.log.logFile << "Results written to " << cire.results->file << "!" << std::endl;
    cire.graph->log.closeFile();
  }

  return 0;
}