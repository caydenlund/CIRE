#include "cire/core/Cire.h"

Cire::Cire(std::string resultFile) : graph(new Graph()), results(new Results(std::move(resultFile))) {}

Cire::~Cire() { delete graph; }

void Cire::setFile(std::string _file) { file = std::move(_file); }

void Cire::setAbstraction(bool _value) { abstraction = _value; }

void Cire::setAbstractionWindow(std::pair<unsigned int, unsigned int> window) { abstractionWindow = window; }

void Cire::setMinDepth(unsigned int depth) { abstractionWindow.first = depth; }

void Cire::setMaxDepth(unsigned int depth) { abstractionWindow.second = depth; }

void Cire::setCollectErrorComponentData(bool _value) const { graph->collectErrorCompData = _value; }

std::map<ir::Node*, ErrorAnalysisResult> Cire::performErrorAnalysis() const {
    if (abstraction) graph->performAbstraction(abstractionWindow.first, abstractionWindow.second);
    std::set<ir::Node*> output_set;
    for (auto& output : graph->outputs) output_set.insert(graph->findVarNode(output));

    return graph->simplifyWithAbstraction(output_set, 0, true);
}
