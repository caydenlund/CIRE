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


  free(cire.graph);
  return 0;
}