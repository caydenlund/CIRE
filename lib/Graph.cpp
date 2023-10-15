#include "../include/Graph.h"

Graph *graph;

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
  for (auto &variable : variables) {
    os << "\t" << variable.first << " : " << *variable.second << std::endl;
  }

  os << "Nodes:" << std::endl;
  for (auto &node : nodes) {
    os << "\t" << *node << std::endl;
  }
}

Node *Graph::findFreeVarNode(string Var) const {
  auto it = inputs.find(Var);
  if (it != inputs.end()) {
    return it->second;
  }

  return nullptr;
}

Node *Graph::findVarNode(string Var) const {
  auto it = variables.find(Var);
  if (it != variables.end()) {
    return it->second;
  }

  return nullptr;
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

void Graph::generateErrExprDriver() {
  std::vector<Node *> next_worklist;
  int curr_depth = 0;
  int next_depth = -1;

  // Iterate all nodes in the worklist
  while(!workList.empty()) {
    Node *node = *workList.begin();
    workList.erase(node);

    int current_depth = node->depth;
    next_depth = current_depth-1;
    // If node contains a constant, add it to completed list as you cannot
    // compute its derivative.
    if(node->type == NodeType::INTEGER ||
       node->type == NodeType::FLOAT ||
       node->type == NodeType::DOUBLE) {
      derivativeComputedNodes.insert(node);
    } else if(derivativeComputedNodes.find(node) != derivativeComputedNodes.end()) {
      // If derivative of node has already been computed, move on.

    } // TODO: Add code to see if parents converge
    else {

    }
  }
}

void Graph::generateErrExpr(Node *node) {

}






