#include"CIRE.h"

#include <utility>

CIRE::CIRE() {
  graph = new Graph();
  results = new Results();
}

CIRE::~CIRE() {
  delete graph;
}

void CIRE::setFile(std::string _file) {
  file = std::move(_file);
}

void CIRE::setAbstaction(bool _value) {
  abstraction = _value;
}

void CIRE::setAbstractionWindow(std::pair<unsigned int, unsigned int> window) {
  abstractionWindow = window;
}

void CIRE::setMinDepth(unsigned int depth) {
  abstractionWindow.first = depth;
}

void CIRE::setMaxDepth(unsigned int depth) {
  abstractionWindow.second = depth;
}

std::map<Node *, std::vector<ibex::Interval>> CIRE::performErrorAnalysis() {
  if (abstraction) {
    graph->performAbstraction(abstractionWindow.first, abstractionWindow.second);
  }
  std::set<Node*> output_set;
  for (auto &output : graph->outputs) {
    output_set.insert(graph->findVarNode(output));
  }

  std::map<Node *, std::vector<ibex::Interval>> optima_map = graph->SimplifyWithAbstraction(output_set, 0, true);

  return optima_map;

}

void show_usage(std::string name) {
  std::cerr << "Usage: " << name << " <option(s)> <input_file>"
            << "Options:\n"
            << "\t-h,--help\t\tShow this help message\n"
            << "\t-a,--abstraction\t\tEnable abstraction\n"
            << "\t-m,--min-depth\t\tSet minimum depth for abstraction\n"
            << "\t-M,--max-depth\t\tSet maximum depth for abstraction\n"
            << "\t-c,--compare\t\tValidate the generated error expression with the one in the file\n"
            << "\t-o,--output\t\tSet the output file\n"
            << std::endl;
}



int main(int argc, char *argv[]) {
  CIRE cire;

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
      } else if((arg == "-c") || (arg == "--compare")) {
        if (i + 1 < argc) {
          cire.graph->setValidationFile(argv[++i]);
        } else {
          std::cerr << "--compare option requires one argument." << std::endl;
          return 1;
        }
      } else if((arg == "-o" || (arg == "--output"))) {
        if (i + 1 < argc) {
          cire.results->setFile(argv[++i]);
        } else {
          std::cerr << "--output option requires one argument." << std::endl;
          return 1;
        }
      } else {
        cire.setFile(argv[i]);
      }
    }
  }

  cire.graph->parse(*cire.file.c_str());
  // print graph input
  std::map<Node *, std::vector<ibex::Interval>> answer = cire.performErrorAnalysis();

  unsigned i = 0;
  // print the answer map
  for (auto const&[node, result] : answer) {
    // This assumes that the map nodes are ordered in the same way as the outputs list nodes which is not
    // always the case
    assert(cire.graph->symbolTables[i]->table[cire.graph->outputs[i]] == node);
    std::cout << *node << " : " << "\n\tOutput: " << result[0] << ","
                                << "\n\tError: " << result[1] << std::endl;
  }

  const auto end = std::chrono::high_resolution_clock::now();
  cire.time_map["Total"] = end - start;
  std::cout << "Time taken: " << cire.time_map["Total"].count() << " seconds" << std::endl;

  cire.results->writeResults(cire.graph->outputs, answer, cire.time_map);

  return 0;
}