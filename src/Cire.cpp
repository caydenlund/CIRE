#include"Cire.h"

Cire::Cire() {
  graph = new Graph();
  results = new Results();
}

Cire::~Cire() {
  delete graph;
}

void Cire::setFile(std::string _file) {
  file = std::move(_file);
}

void Cire::setAbstaction(bool _value) {
  abstraction = _value;
}

void Cire::setAbstractionWindow(std::pair<unsigned int, unsigned int> window) {
  abstractionWindow = window;
}

void Cire::setMinDepth(unsigned int depth) {
  abstractionWindow.first = depth;
}

void Cire::setMaxDepth(unsigned int depth) {
  abstractionWindow.second = depth;
}

std::map<Node *, std::vector<ibex::Interval>> Cire::performErrorAnalysis() {
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
