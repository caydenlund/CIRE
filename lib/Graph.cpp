#include "../include/Graph.h"

Graph *graph;

int SymbolTable::SCOPE_COUNTER = 0;

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

bool Graph::parentsVisited(Node *node) {
  return numParentsOfNode[node] >= node->parents.size();
}

void Graph::derivativeComputingDriver() {
  int next_depth = -1;

  // Iterate all nodes in the worklist
  while(!workList.empty()) {
    Node *node = *workList.begin();
    workList.erase(node);

    int current_depth = node->depth;
    next_depth = current_depth-1;
    // If node contains a constant, add it to completed list as you cannot
    // compute its derivative.
    if(derivativeComputedNodes[current_depth].find(node) != derivativeComputedNodes[current_depth].end()) {
      // If derivative of node has already been computed, move on.
    }
    else if(node->type == NodeType::INTEGER ||
       node->type == NodeType::FLOAT ||
       node->type == NodeType::DOUBLE) {
      derivativeComputedNodes[current_depth].insert(node);
    }
    else if(parentsVisited(node)) {
      derivativeComputing(node);
    }
    else {
      workList.insert(node);
    }

//    printBwdDerivativesIbexExprs();

    std::copy_if(nextWorkList.begin(), nextWorkList.end(), std::inserter(workList, workList.end()), [&next_depth](
            Node *node) { return node->depth == next_depth;});
  }
}

void Graph::derivativeComputing(Node *node) {
  std::vector<Node *> outputList = keys(BwdDerivatives[node]);

  switch (node->type) {
    case DEFAULT:
    case INTEGER:
    case FLOAT:
    case DOUBLE:
    case FREE_VARIABLE:
    case VARIABLE:
      break;
    case UNARY_OP:
      for (Node *outVar : outputList) {
        auto *derivThroughNode = (ibex::ExprNode *)&(*BwdDerivatives[node][outVar] * *getDerivativeWRTChildNode(node, 0));

        BwdDerivatives[((UnaryOp *) node)->Operand][outVar] =
                (ibex::ExprNode *) &(*findWithDefaultInsertion(BwdDerivatives[((UnaryOp *) node)->Operand],
                                                               outVar,
                                                               (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(0.0)) +
                                                               *derivThroughNode);

        // Add child to nextWorkList
        nextWorkList.insert(((UnaryOp *) node)->Operand);
        // Increment number of parents of child that have been processed
        numParentsOfNode[((UnaryOp *) node)->Operand]++;
      }
      break;
    case BINARY_OP:
      for (Node *outVar : outputList) {
        // Computing the backward derivative of outVar with respect to node's children
        auto *derivLeftThroughNode = (ibex::ExprNode *)&(*BwdDerivatives[node][outVar] * *getDerivativeWRTChildNode(node, 0));

        BwdDerivatives[((BinaryOp *) node)->leftOperand][outVar] =
                (ibex::ExprNode *) &(*findWithDefaultInsertion(BwdDerivatives[((BinaryOp *) node)->leftOperand],
                                                               outVar,
                                                               (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(0.0)) +
                                     *derivLeftThroughNode);
//          std::cout << *outVar->getExprNode() << " wrt "
//                    << *((BinaryOp *) node)->leftOperand->getExprNode() << " : "
//                    << *derivLeftThroughNode << std::endl;

        auto *derivRightThroughNode = (ibex::ExprNode *)&(*BwdDerivatives[node][outVar] * *getDerivativeWRTChildNode(node, 1));
        BwdDerivatives[((BinaryOp *) node)->rightOperand][outVar] =
                (ibex::ExprNode *) &(*findWithDefaultInsertion(BwdDerivatives[((BinaryOp *) node)->rightOperand],
                                                               outVar,
                                                               (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(0.0)) +
                                     *derivRightThroughNode);
//          std::cout << *outVar->getExprNode() << " wrt "
//                    << *((BinaryOp *) node)->rightOperand->getExprNode() << " : "
//                    << *derivRightThroughNode << std::endl;

        // Add children to nextWorkList
        nextWorkList.insert(((BinaryOp *) node)->leftOperand);
        nextWorkList.insert(((BinaryOp *) node)->rightOperand);

        // Increment number of parents of children that have been processed
        numParentsOfNode[((BinaryOp *) node)->leftOperand]++;
        numParentsOfNode[((BinaryOp *) node)->rightOperand]++;
      }
      break;
    case TERNARY_OP:
      // TODO: Complete this on adding ternary operations
      break;

  }

  derivativeComputedNodes[node->depth].insert(node);
}

void Graph::printBwdDerivative(Node *outNode, Node *WRTNode) {
  std::cout << *outNode->getExprNode() << " wrt "
            << *WRTNode->getExprNode() << " : "
            << *this->BwdDerivatives[WRTNode][outNode] << std::endl;
}

void Graph::printBwdDerivativesIbexExprs() {
  std::cout << "Backward Derivatives: " << std::endl;
  for (auto &wrtNode : this->BwdDerivatives) {
    for (auto &outputNode : wrtNode.second) {
      printBwdDerivative(outputNode.first, wrtNode.first);
    }
  }
}

void Graph::errorComputingDriver() {
  for(auto &output : outputs) {
    if(errorComputedNodes[findVarNode(output)->depth].find(variables[output]) ==
    errorComputedNodes[findVarNode(output)->depth].end()) {
      errorComputing(findVarNode(output));
    }
  }
}

void Graph::errorComputing(Node *node) {
  switch (node->type) {
    case DEFAULT:
    case INTEGER:
    case FLOAT:
    case DOUBLE:
    case FREE_VARIABLE:
    case VARIABLE:
      break;
    case UNARY_OP:
      if(errorComputedNodes[((UnaryOp *) node)->Operand->depth].find(((UnaryOp *) node)->Operand) ==
         errorComputedNodes[((UnaryOp *) node)->Operand->depth].end()) {
        errorComputing(((UnaryOp *) node)->Operand);
      }
      break;
    case BINARY_OP:
      if(errorComputedNodes[((BinaryOp *) node)->leftOperand->depth].find(((BinaryOp *) node)->leftOperand) ==
         errorComputedNodes[((BinaryOp *) node)->leftOperand->depth].end()) {
        errorComputing(((BinaryOp *) node)->leftOperand);
      }
      if(errorComputedNodes[((BinaryOp *) node)->rightOperand->depth].find(((BinaryOp *) node)->rightOperand) ==
         errorComputedNodes[((BinaryOp *) node)->rightOperand->depth].end()) {
        errorComputing(((BinaryOp *) node)->rightOperand);
      }
      break;
    case TERNARY_OP:
      // TODO: Complete this on adding ternary operations
      break;
  }

  if(derivativeComputedNodes[node->depth].find(node) != derivativeComputedNodes[node->depth].end()) {
    propagateError(node);
  }
  errorComputedNodes[node->depth].insert(node);
}

void Graph::propagateError(Node *node) {
  std::vector<Node *> outputList = keys(BwdDerivatives[node]);

  for (Node *outVar : outputList) {
//    printBwdDerivative(outVar, node);
//    std::cout << node->getAbsoluteError() << std::endl;
//    std::cout << node->getRounding() << std::endl;
    // Generate the error expression by computing the product of the Backward derivative of outVar wrt node and
    // the rounding and noise
    auto expr = (ibex::ExprNode *) &abs((*BwdDerivatives[node][outVar] *
            node->getAbsoluteError() *
            node->getRounding()));

    ErrAccumulator[outVar] =
            (ibex::ExprNode *) &(*findWithDefaultInsertion(ErrAccumulator,
                                                           outVar,
                                                           (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(0.0)) +
                                                           *expr);
    std::cout << "Error Accumulator for " << *outVar->getExprNode() << " : " << *ErrAccumulator[outVar] << std::endl;
  }


}


ibex::ExprNode *getDerivativeWRTChildNode(Node *node, int index) {
  Node *child = node->getChildNode(index);

  switch (node->type) {
    case NodeType::INTEGER:
    case NodeType::FLOAT:
    case NodeType::DOUBLE:
    case NodeType::FREE_VARIABLE:
    case NodeType::VARIABLE:
      return (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(0.0);
    case NodeType::UNARY_OP:
      switch (((UnaryOp*) node)->op) {
        case UnaryOp::SIN:
          return (ibex::ExprNode *) &ibex::cos(*child->getExprNode());
        case UnaryOp::COS:
          return (ibex::ExprNode *) &ibex::sin(-*child->getExprNode());
        case UnaryOp::TAN:
          return (ibex::ExprNode *) &(1.0/ibex::sqr(ibex::cos(*child->getExprNode())));
        case UnaryOp::SINH:
          return (ibex::ExprNode *) &(ibex::exp(*child->getExprNode())-ibex::exp(-*child->getExprNode())/2.0);
        case UnaryOp::COSH:
          return (ibex::ExprNode *) &(ibex::exp(*child->getExprNode())+ibex::exp(-*child->getExprNode())/2.0);
        case UnaryOp::TANH:
          return (ibex::ExprNode *) &(ibex::sinh(*child->getExprNode())/ibex::cosh(*child->getExprNode()));
          case UnaryOp::ASIN:
          return (ibex::ExprNode *) &(1.0/ibex::sqrt(1.0-ibex::sqr(*child->getExprNode())));
        case UnaryOp::ACOS:
          return (ibex::ExprNode *) &(-1.0/ibex::sqrt(1.0-ibex::sqr(*child->getExprNode())));
        case UnaryOp::ATAN:
          return (ibex::ExprNode *) &(1.0/(1.0+ibex::sqr(*child->getExprNode())));
        case UnaryOp::LOG:
          return (ibex::ExprNode *) &(1.0/(*child->getExprNode()*ibex::log(10.0)));
        case UnaryOp::SQRT:
          return (ibex::ExprNode *) &(1.0/(2.0*ibex::sqrt(*child->getExprNode())));
        case UnaryOp::EXP:
          return child->getExprNode();
      }
      break;
    case NodeType::BINARY_OP:
      switch (((BinaryOp*) node)->op) {
        case BinaryOp::ADD:
          return (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(1);
        case BinaryOp::SUB:
          if (index == 0) {
            return (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(1);
          }
          else {
            return (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(-1);
          }
        case BinaryOp::MUL:
          if (index == 0) {
            return ((BinaryOp*) node)->rightOperand->getExprNode();
          }
          else {
            return ((BinaryOp*) node)->leftOperand->getExprNode();
          }
        case BinaryOp::DIV:
          if (index == 0) {
            return (ibex::ExprNode *) &(1.0 / *((BinaryOp*) node)->rightOperand->getExprNode());
          } else if (index == 1) {
            return (ibex::ExprNode *) &(-*((BinaryOp*) node)->leftOperand->getExprNode() /
                                        ibex::sqr(*((BinaryOp*) node)->rightOperand->getExprNode()));
          }
      }
      break;
    case NodeType::TERNARY_OP:
      break;
    default:
      std::cout << "Unknown node type" << std::endl;
      exit(1);
  }

  return nullptr;
}

template<class T1, class T2>
std::vector<T1> keys(std::map<T1, T2> map) {
  std::vector<T1> keys;
  for (auto &key_value : map) {
    keys.push_back(key_value.first);
  }
  return keys;
}

template<class T1, class T2>
T2 findWithDefaultInsertion(std::map<T1, T2> map, T1 key, T2 defaultVal) {
  auto it = map.find(key);
  if (it != map.end()) {
    return it->second;
  }
  else {
    map[key] = defaultVal;
    return defaultVal;
  }
}