#include"CIRE.h"

CIRE::CIRE() {
  graph = new Graph();
}

CIRE::~CIRE() {
  delete graph;
}

void CIRE::setFile(std::string _file) {
  file = _file;
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

ibex::IntervalVector CIRE::performErrorAnalysis() {
  if (abstraction) {
    graph->performAbstraction(abstractionWindow.first, abstractionWindow.second);
  }

  graph->generateExprDriver();
  graph->setupDerivativeComputation();

  graph->errorAnalyzer->derivativeComputingDriver();
  graph->errorComputingDriver();

  graph->ibexInterface->setInputIntervals(graph->inputs);
  graph->ibexInterface->setVariables(graph->inputs, graph->symbolTables[graph->currentScope]->table);
  graph->ibexInterface->setFunction(graph->errorAnalyzer->ErrAccumulator[graph->symbolTables[graph->currentScope]->table[graph->outputs[0]]]);

  return graph->ibexInterface->eval();
}

void show_usage(std::string name) {
  std::cerr << "Usage: " << name << " <option(s)> <input_file>"
            << "Options:\n"
            << "\t-h,--help\t\tShow this help message\n"
            << "\t-a,--abstraction\t\tEnable abstraction\n"
            << "\t-m,--min-depth\t\tSet minimum depth for abstraction\n"
            << "\t-M,--max-depth\t\tSet maximum depth for abstraction\n"
            << std::endl;
}



int main(int argc, char *argv[]) {
  CIRE cire;

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
      } else {
        cire.setFile(argv[i]);
      }
    }
  }

  cire.graph->parse(*cire.file.c_str());
  ibex::IntervalVector answer = cire.performErrorAnalysis();
  std::cout << "Output: " << answer << std::endl;

  return 0;
}