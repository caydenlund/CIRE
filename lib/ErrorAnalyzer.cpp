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

//          std::cout << *outVar->getExprNode() << " wrt "
//                    << *((UnaryOp *) node)->Operand->getExprNode() << " : "
//                    << *derivThroughNode << std::endl;

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

void ErrorAnalyzer::propagateError(Node *node) {
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
//    std::cout << "Error Accumulator for " << *outVar->getExprNode() << " : " << *ErrAccumulator[outVar] << std::endl;
//    std::cout << std::endl;
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