#include"Cire.h"

Cire::Cire(std::string logFile, std::string resultFile) {
  graph = new Graph();
  graph->log.setFile(std::move(logFile));
  graph->log.openFile();
  results = new Results(std::move(resultFile));
}

Cire::~Cire() {
  graph->log.closeFile();
  delete graph;
}

void Cire::setFile(std::string _file) {
  file = std::move(_file);
}

void Cire::setDebugLevel(unsigned int level) {
  debugLevel = level;
}

void Cire::setLogLevel(unsigned int level) {
  logLevel = level;
}

void Cire::setAbstraction(bool _value) {
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

void Cire::setCollectErrorComponentData(bool _value) {
  graph->collect_error_component_data = _value;
}

std::map<Node *, ErrorAnalysisResult> Cire::performErrorAnalysis() {
  if (abstraction) {
    graph->performAbstraction(abstractionWindow.first, abstractionWindow.second);
  }
  std::set<Node*> output_set;
  for (auto &output : graph->outputs) {
    output_set.insert(graph->findVarNode(output));
  }

  return graph->SimplifyWithAbstraction(output_set, 0, true);;
}
