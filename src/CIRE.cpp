#include"CIRE.h"

CIRE::CIRE() {
  graph = new Graph();
}

CIRE::~CIRE() {
  delete graph;
}

int main(int argc, char *argv[]) {
  CIRE cire;

  cire.graph->parse(*argv[1]);
  cire.graph->generateExprDriver();
  cire.graph->setupDerivativeComputation();
  cire.graph->errorAnalyzer->derivativeComputingDriver();

  cire.graph->errorComputingDriver();

  cire.graph->ibexInterface->setInputIntervals(cire.graph->inputs);
  cire.graph->ibexInterface->setVariables(cire.graph->inputs, cire.graph->symbolTables[cire.graph->currentScope]->table);
  cire.graph->ibexInterface->setFunction(cire.graph->errorAnalyzer->ErrAccumulator[cire.graph->symbolTables[cire.graph->currentScope]->table[cire.graph->outputs[0]]]);

  ibex::IntervalVector answer = cire.graph->ibexInterface->eval();
  std::cout << "Output: " << answer << std::endl;

  return 0;
}