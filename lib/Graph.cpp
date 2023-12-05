#include "../include/Graph.h"
#include "lib/parser.h"

int SymbolTable::SCOPE_COUNTER = 0;

Graph::~Graph() {
  for (auto &symbolTable : symbolTables) {
    delete symbolTable.second;
  }
  for (auto &node : nodes) {
    delete node;
  }
  delete errorAnalyzer;
  delete ibexInterface;
}

std::ostream &operator<<(std::ostream &os, const Graph &graph) {
  graph.write(os);
  return os;
}

void Graph::write(std::ostream &os) const {
  os << "Graph:" << std::endl;
  os << "Inputs:" << std::endl;
  for (auto &input : inputs) {
    os << "\t" << input.first << " : " << *input.second << std::endl;
  }
  os << "Outputs:" << std::endl;
  for (auto &output : outputs) {
    os << "\t" << output << std::endl;
  }
  os << "Variables:" << std::endl;
  for (auto &variable : symbolTables.find(currentScope)->second->table) {
    os << "\t" << variable.first << " : " << *variable.second << std::endl;
  }

  os << "Nodes:" << std::endl;
  for (auto &node : nodes) {
    os << "\t" << *node << std::endl;
  }
}

void Graph::createNewSymbolTable() {
  currentScope = SymbolTable::SCOPE_COUNTER;
  symbolTables[currentScope] = new SymbolTable();
}

Node *Graph::findFreeVarNode(string Var) const {
  auto it = inputs.find(Var);
  if (it != inputs.end()) {
    return it->second;
  }

  return nullptr;
}

Node *Graph::findVarNode(string Var) const {
  auto it = symbolTables.find(currentScope)->second->table.find(Var);
  if (it != symbolTables.find(currentScope)->second->table.end()) {
    return it->second;
  }

  return nullptr;
}

void Graph::setupDerivativeComputation() {
  // Set up output
  // Assuming there is only one output
  // TODO: Change this for multiple outputs
  errorAnalyzer->workList.insert(symbolTables[currentScope]->table[outputs[0]]);
  errorAnalyzer->BwdDerivatives[symbolTables[currentScope]->table[outputs[0]]][symbolTables[currentScope]->table[outputs[0]]] =
          (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(1);
  errorAnalyzer->numParentsOfNode[symbolTables[currentScope]->table[outputs[0]]] =
          symbolTables[currentScope]->table[outputs[0]]->parents.size();
}

void Graph::errorComputingDriver() {
  for(auto &output : outputs) {
    if(errorAnalyzer->errorComputedNodes[findVarNode(output)->depth].find(symbolTables.find(currentScope)->second->table[output]) ==
        errorAnalyzer->errorComputedNodes[findVarNode(output)->depth].end()) {
      errorAnalyzer->errorComputing(findVarNode(output));
    }
  }

  errorAnalyzer->ErrAccumulator[symbolTables[currentScope]->table[outputs[0]]] =
          (ibex::ExprNode*) &(*errorAnalyzer->ErrAccumulator[symbolTables[currentScope]->table[outputs[0]]] *
          pow(2, -53));
}

// Generates Expressions corresponding to all no bottom up
void Graph::generateExprDriver() {
  for (auto &output : outputs) {
    generateExpr(findVarNode(output));
  }
}

void Graph::generateExpr(Node *node) {
  switch (node->type) {
    case NodeType::INTEGER:
      // Already has an expression
      break;
    case NodeType::FLOAT:
      // Already has an expression
      break;
    case NodeType::DOUBLE:
      // Already has an expression
      break;
    case NodeType::FREE_VARIABLE:
      // Already has an expression
      break;
    case NodeType::VARIABLE:
      // Already has an expression
      break;
    case NodeType::UNARY_OP:
      generateExpr(((UnaryOp *)node)->Operand);
      ((UnaryOp *)node)->expr = (ibex::ExprUnaryOp *)&node->generateSymExpr();
//      std::cout << "UnaryOp: " << ((UnaryOp *)node)->expr << std::endl;
      break;
    case NodeType::BINARY_OP:
      generateExpr(((BinaryOp *)node)->leftOperand);
      generateExpr(((BinaryOp *)node)->rightOperand);
      ((BinaryOp *)node)->expr = (ibex::ExprBinaryOp *)&node->generateSymExpr();
//      std::cout << "BinaryOp: " << *((BinaryOp *)node)->expr << std::endl;
      break;
    case NodeType::TERNARY_OP:
      generateExpr(((TernaryOp *)node)->leftOperand);
      generateExpr(((TernaryOp *)node)->middleOperand);
      generateExpr(((TernaryOp *)node)->rightOperand);
      // Ibex does not have a TernaryOp
      break;
    default:
      std::cout << "Unknown node type" << std::endl;
      break;
  }
}

std::set<Node*> Graph::selectNodesForAbstraction(unsigned int maxDepth,
                                                    unsigned int boundMinDepth,
                                                    unsigned int boundMaxDepth) {
  assert(boundMinDepth <= boundMaxDepth && boundMaxDepth <= maxDepth);
  std::set<Node*> nodesToAbstract;
  if (boundMinDepth == boundMaxDepth && boundMaxDepth <= maxDepth) {
    return depthTable[boundMinDepth];
  }
}

void Graph::performAbstraction(unsigned int boundMinDepth, unsigned int boundMaxDepth) {
  // Get max depth using keys in depthTable
  unsigned int maxDepth = depthTable.rbegin()->first;

  auto candidateNodes = selectNodesForAbstraction(maxDepth, boundMinDepth, boundMaxDepth);

  // Print candidate nodes
  std::cout << "Candidate Nodes:" << std::endl;
  for (auto &node : candidateNodes) {
    std::cout << "\t" << *node << std::endl;
  }
}

int Graph::parse(const char &f) {
  yydebug = 0;
  yyin = fopen(&f, "r");
  if(!yyin) {
    std::cout << "Bad Input. Non-existant file" << std::endl;
    return -1;
  }

  do {
    std::cout << "Parsing..." << std::endl;
    createNewSymbolTable();
    if(yyparse(this)) {
      std::cout << "Parsing failed" << std::endl;
      return 0;
    }
    else {
      std::cout << "Parsing successful" << std::endl;
    }
  } while (!feof(yyin));

//  std::cout << *graph << std::endl;

  return 0;
}
