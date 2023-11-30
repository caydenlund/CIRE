#include "../include/ErrorAnalyzer.h"

bool ErrorAnalyzer::parentsVisited(Node *node) {
  return numParentsOfNode[node] >= node->parents.size();
}

void ErrorAnalyzer::derivativeComputingDriver() {
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

    std::copy_if(nextWorkList.begin(), nextWorkList.end(), std::inserter(workList, workList.end()), [&next_depth](
            Node *node) { return node->depth == next_depth;});
  }
//  printBwdDerivativesIbexExprs();
//  std::cout << std::endl;
}

void ErrorAnalyzer::derivativeComputing(Node *node) {
  std::vector<Node *> outputList = keys(BwdDerivatives[node]);
  for (Node *outVar : outputList) {
    assert(BwdDerivatives[node][outVar] != nullptr && "Derivative of output wrt node should have been computed\n");

    Node *Operand, *leftOperand, *rightOperand;
    ibex::ExprNode *derivThroughNode, *derivLeftThroughNode, *derivRightThroughNode;
    switch (node->type) {
      case DEFAULT:
      case INTEGER:
      case FLOAT:
      case DOUBLE:
      case FREE_VARIABLE:
      case VARIABLE:
        break;
      case UNARY_OP:
        Operand = ((UnaryOp *) node)->Operand;
        derivThroughNode = (ibex::ExprNode *) &product(*BwdDerivatives[node][outVar],
                                                      *getDerivativeWRTChildNode(node, 0)).simplify(0);

        if (contains(BwdDerivatives[Operand], outVar)) {
          BwdDerivatives[Operand][outVar] = (ibex::ExprNode *) &(*BwdDerivatives[Operand][outVar] + *derivThroughNode);
        } else {
          BwdDerivatives[Operand][outVar] = (ibex::ExprNode *) &(*derivThroughNode);
        }

//        std::cout << *node->getExprNode() << " wrt "
//                  << *Operand->getExprNode() << " : "
//                  << *derivThroughNode << std::endl;
//        std::cout << "Derivative so far of " << *outVar->getExprNode() << " wrt "
//                  << *Operand->getExprNode() << " : "
//                  << *BwdDerivatives[Operand][outVar] << std::endl;

        // Add child to nextWorkList
        nextWorkList.insert(Operand);
        // Increment number of parents of child that have been processed
        numParentsOfNode[Operand]++;
        break;
      case BINARY_OP:
        // Computing the backward derivative of outVar with respect to node's children
        leftOperand = ((BinaryOp *) node)->leftOperand;
        derivLeftThroughNode = (ibex::ExprNode *) &product(*BwdDerivatives[node][outVar],
                                                          *getDerivativeWRTChildNode(node, 0)).simplify(0);

        if (contains(BwdDerivatives[leftOperand], outVar)) {
          BwdDerivatives[leftOperand][outVar] = (ibex::ExprNode *) &(*BwdDerivatives[leftOperand][outVar] +
                                                                     *derivLeftThroughNode);
        } else {
          BwdDerivatives[leftOperand][outVar] = (ibex::ExprNode *) &(*derivLeftThroughNode);
        }

//        std::cout << *node->getExprNode() << " wrt "
//                  << *leftOperand->getExprNode() << " : "
//                  << *derivLeftThroughNode << std::endl;
//        std::cout << "Derivative so far of " << *outVar->getExprNode() << " wrt "
//                  << *leftOperand->getExprNode() << " : "
//                  << *BwdDerivatives[leftOperand][outVar] << std::endl;

        rightOperand = ((BinaryOp *) node)->rightOperand;
        derivRightThroughNode = (ibex::ExprNode *) &product(*BwdDerivatives[node][outVar],
                                                           *getDerivativeWRTChildNode(node, 1)).simplify(0);

        if (contains(BwdDerivatives[rightOperand], outVar)) {
          BwdDerivatives[rightOperand][outVar] = (ibex::ExprNode *) &(*BwdDerivatives[rightOperand][outVar] +
                                                                      *derivRightThroughNode);
        } else {
          BwdDerivatives[rightOperand][outVar] = (ibex::ExprNode *) &(*derivRightThroughNode);
        }

//        std::cout << *node->getExprNode() << " wrt "
//                  << *rightOperand->getExprNode() << " : "
//                  << *derivRightThroughNode << std::endl;
//        std::cout << "Derivative so far of " << *outVar->getExprNode() << " wrt "
//                  << *rightOperand->getExprNode() << " : "
//                  << *BwdDerivatives[rightOperand][outVar] << std::endl;

        // Add children to nextWorkList
        nextWorkList.insert(leftOperand);
        nextWorkList.insert(rightOperand);

        // Increment number of parents of children that have been processed
        numParentsOfNode[leftOperand]++;
        numParentsOfNode[rightOperand]++;
        break;
      case TERNARY_OP:
        // TODO: Complete this on adding ternary operations
        break;
    }
  }

  derivativeComputedNodes[node->depth].insert(node);
}

void ErrorAnalyzer::printBwdDerivative(Node *outNode, Node *WRTNode) {
  std::cout << *outNode->getExprNode() << " wrt "
            << *WRTNode->getExprNode() << " : "
            << *this->BwdDerivatives[WRTNode][outNode] << std::endl;
}

void ErrorAnalyzer::printBwdDerivativesIbexExprs() {
  std::cout << "Backward Derivatives: " << std::endl;
  for (auto &wrtNode : this->BwdDerivatives) {
    for (auto &outputNode : wrtNode.second) {
      printBwdDerivative(outputNode.first, wrtNode.first);
    }
  }
}



void ErrorAnalyzer::errorComputing(Node *node) {
  Node *Operand, *leftOperand, *rightOperand;

  switch (node->type) {
    case DEFAULT:
    case INTEGER:
    case FLOAT:
    case DOUBLE:
    case FREE_VARIABLE:
    case VARIABLE:
      break;
    case UNARY_OP:
      Operand = ((UnaryOp *) node)->Operand;
      if(errorComputedNodes[Operand->depth].find(Operand) ==
         errorComputedNodes[Operand->depth].end()) {
        errorComputing(Operand);
      }
      break;
    case BINARY_OP:
      leftOperand = ((BinaryOp *) node)->leftOperand;
      if(errorComputedNodes[leftOperand->depth].find(leftOperand) ==
         errorComputedNodes[leftOperand->depth].end()) {
        errorComputing(leftOperand);
      }

      rightOperand = ((BinaryOp *) node)->rightOperand;
      if(errorComputedNodes[rightOperand->depth].find(rightOperand) ==
         errorComputedNodes[rightOperand->depth].end()) {
        errorComputing(rightOperand);
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

void ErrorAnalyzer::propagateError(Node *node) {
  std::vector<Node *> outputList = keys(BwdDerivatives[node]);

  for (Node *outVar : outputList) {
//    printBwdDerivative(outVar, node);
//    std::cout << node->getAbsoluteError() << std::endl;
//    std::cout << node->getRounding() << std::endl;
    // Generate the error expression by computing the product of the Backward derivative of outVar wrt node and
    // the rounding and noise
    auto local_error = (ibex::ExprNode *) &product(node->getAbsoluteError(), node->getRounding()).simplify(0);
    auto expr = (ibex::ExprNode *) &abs(product(*BwdDerivatives[node][outVar],
                                                *local_error)).simplify(0);

    if (contains(ErrAccumulator, outVar)) {
      ErrAccumulator[outVar] = (ibex::ExprNode *) &(*ErrAccumulator[outVar] + *expr);
    } else {
      ErrAccumulator[outVar] = (ibex::ExprNode *) &(*expr);
    }

    std::cout << "Error Accumulator for " << *outVar->getExprNode() << " : " << *ErrAccumulator[outVar] << std::endl;
    std::cout << std::endl;
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
        case UnaryOp::NEG:
          return (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(-1);
        case UnaryOp::SIN:
          return (ibex::ExprNode *) &cos(*child->getExprNode());
        case UnaryOp::COS:
          return (ibex::ExprNode *) &sin(-*child->getExprNode());
        case UnaryOp::TAN:
          return (ibex::ExprNode *) &(1.0/sqr(cos(*child->getExprNode())));
        case UnaryOp::SINH:
          return (ibex::ExprNode *) &(exp(*child->getExprNode())-exp(-*child->getExprNode())/2.0);
        case UnaryOp::COSH:
          return (ibex::ExprNode *) &(exp(*child->getExprNode())+exp(-*child->getExprNode())/2.0);
        case UnaryOp::TANH:
          return (ibex::ExprNode *) &(sinh(*child->getExprNode())/cosh(*child->getExprNode()));
        case UnaryOp::ASIN:
          return (ibex::ExprNode *) &(1.0/sqrt(1.0-sqr(*child->getExprNode())));
        case UnaryOp::ACOS:
          return (ibex::ExprNode *) &(-1.0/sqrt(1.0-sqr(*child->getExprNode())));
        case UnaryOp::ATAN:
          return (ibex::ExprNode *) &(1.0/(1.0+sqr(*child->getExprNode())));
        case UnaryOp::LOG:
          return (ibex::ExprNode *) &(1.0/(*child->getExprNode()*log(10.0)));
        case UnaryOp::SQRT:
          return (ibex::ExprNode *) &(1.0/(2.0*sqrt(*child->getExprNode())));
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
                                        sqr(*((BinaryOp*) node)->rightOperand->getExprNode()));
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
bool contains(std::map<T1, T2> map, T1 key) {
  return map.find(key) != map.end();
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