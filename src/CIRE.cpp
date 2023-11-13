#include"CIRE.h"

CIRE::CIRE() {
  graph = new Graph();
}

int CIRE::parse(const char &f) {
  std::ofstream myfile;
  yydebug = 0;
  yyin = fopen(&f, "r");
  if(!yyin) {
    std::cout << "Bad Input.Non-existant file" << std::endl;
    return -1;
  }

  do {
    std::cout << "Parsing..." << std::endl;
    graph->createNewSymbolTable();
    yyparse(graph);
  } while (!feof(yyin));

//  std::cout << *graph << std::endl;

  return 0;
}

int main(int argc, char *argv[]) {
  CIRE cire;

  cire.parse(*argv[1]);

  cire.graph->generateExprDriver();


  // Set up output
  // Assuming there is only one output
  // TODO: Change this for multiple outputs
  cire.graph->workList.insert(cire.graph->variables[cire.graph->outputs[0]]);
  cire.graph->BwdDerivatives[cire.graph->variables[cire.graph->outputs[0]]][cire.graph->variables[cire.graph->outputs[0]]] =
          (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(1);
  cire.graph->numParentsOfNode[cire.graph->variables[cire.graph->outputs[0]]] =
          cire.graph->variables[cire.graph->outputs[0]]->parents.size();

  cire.graph->derivativeComputingDriver();

  cire.graph->errorComputingDriver();



  // Print Intervals from inputs
  double x[2][2];
  int i = 0;
  for (auto &input : cire.graph->inputs) {
    std::cout << input.first << ": " << *input.second->var << std::endl;
    x[i][0] = input.second->var->lb();
    x[i][1] = input.second->var->ub();
    i++;
  }
  ibex::IntervalVector *iv = new ibex::IntervalVector(2, x);
  std::cout << "Input: " << *iv << std::endl;

  std::cout << "Function:" << *cire.graph->ErrAccumulator[cire.graph->variables[cire.graph->outputs[0]]] << std::endl;
  // Print variables
  auto *variables = new ibex::Array<const ibex::ExprSymbol>();
  std::cout << "Variables: " << std::endl;
  for (auto &input : cire.graph->inputs) {
    std::cout << input.first << ": " << *cire.graph->variables[input.first]->getExprNode() << std::endl;
    variables->add(*(ibex::ExprSymbol*) cire.graph->variables[input.first]->getExprNode());
  }
  auto *temp = new ibex::Function(*variables,
                                  *cire.graph->ErrAccumulator[cire.graph->variables[cire.graph->outputs[0]]]);

  ibex::IntervalVector answer = temp->eval(*iv) * pow(2, -53);
  std::cout << "Output: " << answer << std::endl;

  free(cire.graph);
  return 0;
}