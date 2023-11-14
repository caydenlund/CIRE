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

  // Print Intervals from inputs
  double x[2][2];
  int i = 0;
  for (auto &input : cire.graph->inputs) {
//    std::cout << input.first << ": " << *input.second->var << std::endl;
    x[i][0] = input.second->var->lb();
    x[i][1] = input.second->var->ub();
    i++;
  }
  auto *iv = new ibex::IntervalVector(2, x);
  std::cout << "Input: " << *iv << std::endl;
  std::cout << "Function:" << *cire.graph->errorAnalyzer->ErrAccumulator[cire.graph->symbolTables[cire.graph->currentScope]->table[cire.graph->outputs[0]]] << std::endl;
  // Print variables
  auto *variables = new ibex::Array<const ibex::ExprSymbol>();
  std::cout << "Variables: " << std::endl;
  for (auto &input : cire.graph->inputs) {
    std::cout << input.first << ": " << *cire.graph->symbolTables[cire.graph->currentScope]->table[input.first]->getExprNode() << std::endl;
    variables->add(*(ibex::ExprSymbol*) cire.graph->symbolTables[cire.graph->currentScope]->table[input.first]->getExprNode());
  }
  auto *temp = new ibex::Function(*variables,
                                  *cire.graph->errorAnalyzer->ErrAccumulator[cire.graph->symbolTables[cire.graph->currentScope]->table[cire.graph->outputs[0]]]);

  ibex::IntervalVector answer = temp->eval(*iv) * pow(2, -53);
  std::cout << "Output: " << answer << std::endl;

  return 0;
}