#include "../include/Graph.h"

#include <utility>
#include "lib/parser.h"

int SymbolTable::SCOPE_COUNTER = 0;

Graph::Graph(string logFile) {
  log.setFile(std::move(logFile));
}

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

void Graph::setValidationFile(std::string _validationFile) {
  validationFile = std::move(_validationFile);
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

  os << "Depth Table:" << std::endl;
  for (auto &depth : depthTable) {
    os << "\t" << depth.first << " : ";
    for (auto &node : depth.second) {
      os << *node << " ";
    }
    os << std::endl;
  }
}

void Graph::createNewSymbolTable() {
  currentScope = SymbolTable::SCOPE_COUNTER;
  symbolTables[currentScope] = new SymbolTable();
}

void Graph::generateIbexSymbols() {
  for (auto &input : inputs) {
    assert(symbolTables[currentScope]->table[input.first]->isVariable() && "Input is not a variable node");
    ((VariableNode *)symbolTables[currentScope]->table[input.first])->variable = &(ibex::ExprSymbol::new_(input.first.c_str()));
    symbolTables[currentScope]->table[input.first]->setAbsoluteError(&ibex::ExprConstant::new_scalar(input.second->var->ub() * pow(2, -53)));
  }


  for (auto &node: nodes) {
    switch (node->type) {
      case NodeType::INTEGER: {
        ((Integer*)node)->value = &(ibex::ExprConstant::new_scalar(((Integer*)node)->val));
        node->setAbsoluteError(&ibex::ExprConstant::new_scalar(0.0));
        break;
      }
      case NodeType::FLOAT: {
        ((Float*)node)->value = &(ibex::ExprConstant::new_scalar(((Float*)node)->val));
        node->setAbsoluteError(&ibex::ExprConstant::new_scalar(((Float*)node)->val * pow(2, -24)));
        break;
      }
      case NodeType::DOUBLE: {
        ((Double*)node)->value = &(ibex::ExprConstant::new_scalar(((Double*)node)->val));
        node->setAbsoluteError(&ibex::ExprConstant::new_scalar(((Double*)node)->val * pow(2, -53)));
        break;
      }
      case NodeType::FREE_VARIABLE: {
        node->setAbsoluteError(&ibex::ExprConstant::new_scalar(((FreeVariable*)node)->var->ub() * pow(2, -53)));
        break;
      }
      case NodeType::VARIABLE: // The absoluteError has already been set in the previous inputs for loop
      // Following nodes do not have an absolute error. Only Constants and FreeVariables have an absolute error
      case NodeType::UNARY_OP:
      case NodeType::BINARY_OP:
      case NodeType::TERNARY_OP:
      case NodeType::DEFAULT: {
        break;
      }
    }
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
  auto it = symbolTables.find(currentScope)->second->table.find(Var);
  if (it != symbolTables.find(currentScope)->second->table.end()) {
    return it->second;
  }

  return nullptr;
}

void Graph::setupDerivativeComputation(std::set<Node*> candidate_nodes) {
  // Set up output
  // Get the max depth of the candidate_nodes
  unsigned int max_depth = 0;
  for (auto &node : candidate_nodes) {
    if (node->depth > max_depth) {
      max_depth = node->depth;
    }
  }

  errorAnalyzer->derivativeComputedNodes.clear();
  errorAnalyzer->errorComputedNodes.clear();
  errorAnalyzer->numParentsOfNode.clear();
  errorAnalyzer->parentsOfNode.clear();
  errorAnalyzer->BwdDerivatives.clear();
  errorAnalyzer->typeCastRnd.clear();
  errorAnalyzer->ErrAccumulator.clear();

  // Insert candidate_nodes with max depth into worklist
  for (auto &node : candidate_nodes) {
    if (node->depth == max_depth) {
      errorAnalyzer->workList.insert(node);
    }
  }

  // Set BwdDerivatives of each candidate_node (output node) with respect to itself to 1
  for (auto &node : candidate_nodes) {
    errorAnalyzer->BwdDerivatives[node][node] = (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(1);
    errorAnalyzer->typeCastRnd[node][node] = (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(0);
  }

  // Set numParentsOfNode of each node to the number of parents it has
  for (auto &node : candidate_nodes) {
    errorAnalyzer->numParentsOfNode[node] = node->parents.size();
  }
}

// Generates Expressions corresponding to all candidate_nodes bottom up
void Graph::generateExprDriver(const std::set<Node *> &candidate_nodes) {
  // Map from depth to nodes at that depth whose expression has been generated. Similar to "reachable" in Satire
  std::map<int, std::set<Node *>> generatedExprsAtDepth;

  // Map from Ibex::ExprNode to the Nodes that have that expression
  // Common Subexpression Elimination Table
  // This keeps track of nodes that have the same expression and can be replaced by a single node
  std::map<ibex::ExprNode *, std::set<Node *>> cseTable;

  if (debugLevel > 1) {
    std::cout << "Generating Expressions..." << std::endl;
  }
  if (logLevel > 1) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Generating Expressions..." << std::endl;
  }

  for (auto &node : candidate_nodes) {
    if (debugLevel > 3) {
      std::cout << "Processing Node " << node->id << std::endl;
    }
    if (logLevel > 3) {
      log.logFile << "Processing Node " << node->id << std::endl;
    }
    if (generatedExprsAtDepth[node->depth].find(node) == generatedExprsAtDepth[node->depth].end()) {
      generateExpr(node, generatedExprsAtDepth, cseTable);
    }
    if (debugLevel > 3) {
      std::cout << "Node " << node->id << " processed." << std::endl;
    }
    if (logLevel > 3) {
      log.logFile << "Node " << node->id << " processed." << std::endl;
    }
  }

  if (debugLevel > 1) {
    std::cout << "Expressions Generated!" << std::endl;
  }
  if (logLevel > 1) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Expressions Generated!" << std::endl;
  }
}

void Graph::generateExpr(Node *node, std::map<int, std::set<Node *>> &generatedExprsAtDepth,
                         std::map<ibex::ExprNode *, std::set<Node *>> &cseTable) {
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
      // Does not have an associated Ibex Expression. Instead has interval
      break;
    case NodeType::VARIABLE:
      // Already has an expression
      break;
    case NodeType::UNARY_OP:
      if(generatedExprsAtDepth[((UnaryOp *)node)->Operand->depth].find(((UnaryOp *)node)->Operand) ==
         generatedExprsAtDepth[((UnaryOp *)node)->Operand->depth].end()) {
        generateExpr(((UnaryOp *)node)->Operand, generatedExprsAtDepth, cseTable);
      }
      ((UnaryOp *)node)->expr = (ibex::ExprUnaryOp *)&node->generateSymExpr();
      errorAnalyzer->parentsOfNode[((UnaryOp *)node)->Operand].insert(node);
      if (debugLevel > 3) {
        std::cout << "Node " << node->id << " processed." << std::endl;
        if (debugLevel > 4) {
          std::cout << "UnaryOp: " << *((UnaryOp *) node)->expr << std::endl;
        }
      }
      if (logLevel > 3) {
        log.logFile << "Node " << node->id << " processed." << std::endl;
        if (logLevel > 4) {
          assert(log.logFile.is_open() && "Log file not open");
          log.logFile << "UnaryOp: " << *((UnaryOp *) node)->expr << std::endl;
        }
      }
      break;
    case NodeType::BINARY_OP:
      if(generatedExprsAtDepth[((BinaryOp *)node)->leftOperand->depth].find(((BinaryOp *)node)->leftOperand) ==
         generatedExprsAtDepth[((BinaryOp *)node)->leftOperand->depth].end()) {
        generateExpr(((BinaryOp *)node)->leftOperand, generatedExprsAtDepth, cseTable);
      }
      if(generatedExprsAtDepth[((BinaryOp *)node)->rightOperand->depth].find(((BinaryOp *)node)->rightOperand) ==
         generatedExprsAtDepth[((BinaryOp *)node)->rightOperand->depth].end()) {
        generateExpr(((BinaryOp *)node)->rightOperand, generatedExprsAtDepth, cseTable);
      }
      ((BinaryOp *)node)->expr = (ibex::ExprBinaryOp *)&node->generateSymExpr();
      errorAnalyzer->parentsOfNode[((BinaryOp *)node)->leftOperand].insert(node);
      errorAnalyzer->parentsOfNode[((BinaryOp *)node)->rightOperand].insert(node);
      if (debugLevel > 3) {
        std::cout << "Node " << node->id << " processed." << std::endl;
        if (debugLevel > 4) {
          std::cout << "BinaryOp: " << *((BinaryOp *) node)->expr << std::endl;
        }
      }
      if (logLevel > 3) {
        log.logFile << "Node " << node->id << " processed." << std::endl;
        if (logLevel > 4) {
          assert(log.logFile.is_open() && "Log file not open");
          log.logFile << "BinaryOp: " << *((BinaryOp *) node)->expr << std::endl;
        }
      }
      break;
    case NodeType::TERNARY_OP:
      if(generatedExprsAtDepth[((TernaryOp *)node)->leftOperand->depth].find(((TernaryOp *)node)->leftOperand) ==
         generatedExprsAtDepth[((TernaryOp *)node)->leftOperand->depth].end()) {
        generateExpr(((TernaryOp *)node)->leftOperand, generatedExprsAtDepth, cseTable);
      }
      if(generatedExprsAtDepth[((TernaryOp *)node)->middleOperand->depth].find(((TernaryOp *)node)->middleOperand) ==
         generatedExprsAtDepth[((TernaryOp *)node)->middleOperand->depth].end()) {
        generateExpr(((TernaryOp *)node)->middleOperand, generatedExprsAtDepth, cseTable);
      }
      if(generatedExprsAtDepth[((TernaryOp *)node)->rightOperand->depth].find(((TernaryOp *)node)->rightOperand) ==
         generatedExprsAtDepth[((TernaryOp *)node)->rightOperand->depth].end()) {
        generateExpr(((TernaryOp *)node)->rightOperand, generatedExprsAtDepth, cseTable);
      }
      // Ibex does not have a TernaryOp, so we split the Op into two BinaryOps
      ((TernaryOp *)node)->expr = (ibex::ExprBinaryOp *)&node->generateSymExpr();
      errorAnalyzer->parentsOfNode[((TernaryOp *)node)->leftOperand].insert(node);
      errorAnalyzer->parentsOfNode[((TernaryOp *)node)->middleOperand].insert(node);
      errorAnalyzer->parentsOfNode[((TernaryOp *)node)->rightOperand].insert(node);
      if (debugLevel > 3) {
        std::cout << "Node " << node->id << " processed." << std::endl;
        if (debugLevel > 4) {
          std::cout << "TernaryOp: " << *((TernaryOp *) node)->expr << std::endl;
        }
      }
      if (logLevel > 3) {
        log.logFile << "Node " << node->id << " processed." << std::endl;
        if (logLevel > 4) {
          assert(log.logFile.is_open() && "Log file not open");
          log.logFile << "TernaryOp: " << *((TernaryOp *) node)->expr << std::endl;
        }
      }
      break;
    default:
      std::cout << "Unknown node type" << std::endl;
      exit(1);
  }

  // Update the map tracking processed nodes
  generatedExprsAtDepth[node->depth].insert(node);

  // Common sub expression elimination phase
  // 1) Find all subexpressions similar to the current node
  std::vector<Node *> nodes_to_merge;
  for (auto &n: cseTable[node->getExprNode()]) {
    // Ensuring n and node are not the same nodes
    if(n != node) {
      // Check if all children of n and node are the same
      switch (n->type) {
        case NodeType::INTEGER:
          if (node->isInteger())
            nodes_to_merge.push_back(n);
          break;
        case NodeType::FLOAT:
          if (node->isFloat())
            nodes_to_merge.push_back(n);
          break;
        case NodeType::DOUBLE:
          if (node->isDouble())
            nodes_to_merge.push_back(n);
          break;
        case NodeType::FREE_VARIABLE:
          if (node->isFreeVariable())
            nodes_to_merge.push_back(n);
          break;
        case NodeType::VARIABLE:
          if (node->isVariable())
            nodes_to_merge.push_back(n);
          break;
        case NodeType::UNARY_OP:
          if (node->isUnaryOp() && ((UnaryOp*)n)->Operand == ((UnaryOp*)node)->Operand) {
            nodes_to_merge.push_back(n);
          }
          break;
        case NodeType::BINARY_OP:
          if (node->isBinaryOp() && ((BinaryOp*)n)->leftOperand == ((BinaryOp*)node)->leftOperand &&
              ((BinaryOp*)n)->rightOperand == ((BinaryOp*)node)->rightOperand) {
            nodes_to_merge.push_back(n);
          }
          break;
        case NodeType::TERNARY_OP:
          if (node->isTernaryOp() && ((TernaryOp*)n)->leftOperand == ((TernaryOp*)node)->leftOperand &&
              ((TernaryOp*)n)->middleOperand == ((TernaryOp*)node)->middleOperand &&
              ((TernaryOp*)n)->rightOperand == ((TernaryOp*)node)->rightOperand) {
            nodes_to_merge.push_back(n);
          }
          break;
        default:
          std::cout << "Unknown node type" << std::endl;
          exit(1);
      }
    }
  }

  // 2) Merge all similar subexpressions
  if(nodes_to_merge.empty()) {
    cseTable[node->getExprNode()].insert(node);
  } else {
    if(debugLevel > 2) {
      std::cout << "Found " << nodes_to_merge.size() << " nodes to merge with node " << node->id << std::endl;
    }
    for(auto &n: nodes_to_merge) {
      // TODO: if n in not in the output set, merge n into node
      node = mergeNodes(n, node, errorAnalyzer->parentsOfNode);
      cseTable[node->getExprNode()].erase(n);
      delete n;
    }
  }

}

// Merges node1 into node2 by
//    Updating the parents of node2 to include the parents of node1
//    Updating the children of the parents of node2 to point to node2 if they point to node1
//    Removing node1 from the depthTable
//    Removing node1 from the symbol table
//    Merging the parentsOfNode entries of node1 and node2
//    Removing node1 from parentsOfNode
Node *Graph::mergeNodes(Node *node1, Node *node2, std::map<Node *, std::set<Node *>> &parentsOfNode) {
  // Set union of parents of node1 and node2
  std::set<Node *> new_parents;
  std::set_union(parentsOfNode[node1].begin(), parentsOfNode[node1].end(),
                 parentsOfNode[node2].begin(), parentsOfNode[node2].end(),
                 std::inserter(new_parents, new_parents.end()));

  // Update the children of the new_parents set to point to node2 of they point to node1
  for (auto &par: new_parents) {
    switch(par->type) {
      case NodeType::INTEGER:
      case NodeType::FLOAT:
      case NodeType::DOUBLE:
      case NodeType::FREE_VARIABLE:
      case NodeType::VARIABLE:
        break;
      case NodeType::UNARY_OP:
        if (((UnaryOp*)par)->Operand == node1) {
          ((UnaryOp*)par)->Operand = node2;
        }
        break;
      case NodeType::BINARY_OP:
        if (((BinaryOp*)par)->leftOperand == node1) {
          ((BinaryOp*)par)->leftOperand = node2;
        }
        if (((BinaryOp*)par)->rightOperand == node1) {
          ((BinaryOp*)par)->rightOperand = node2;
        }
        break;
      case NodeType::TERNARY_OP:
        if (((TernaryOp*)par)->leftOperand == node1) {
          ((TernaryOp*)par)->leftOperand = node2;
        }
        if (((TernaryOp*)par)->middleOperand == node1) {
          ((TernaryOp*)par)->middleOperand = node2;
        }
        if (((TernaryOp*)par)->rightOperand == node1) {
          ((TernaryOp*)par)->rightOperand = node2;
        }
        break;
      default:
        std::cout << "Unknown node type" << std::endl;
        exit(1);
    }
  }

  node2->parents = new_parents;

  // Cleanup of node1
  depthTable[node1->depth].erase(node1);

  // Clear symbol table entry for node1
  for (auto it = symbolTables[currentScope]->table.begin(); it != symbolTables[currentScope]->table.end(); it++) {
    if (it->second == node1) {
      symbolTables[currentScope]->table.erase(it);
      break;
    }
  }

  // Merge parentsOfNode entries
  parentsOfNode[node2].insert(parentsOfNode[node1].begin(), parentsOfNode[node1].end());

  // Update parentsOfNode for children of `node2` by removing `node1`
  switch (node2->type) {
    case NodeType::INTEGER:
    case NodeType::FLOAT:
    case NodeType::DOUBLE:
    case NodeType::FREE_VARIABLE:
    case NodeType::VARIABLE:
      break;
    case NodeType::UNARY_OP:
      parentsOfNode[((UnaryOp*)node2)->Operand].erase(node1);
      break;
    case NodeType::BINARY_OP:
      parentsOfNode[((BinaryOp*)node2)->leftOperand].erase(node1);
      parentsOfNode[((BinaryOp*)node2)->rightOperand].erase(node1);
      break;
    case NodeType::TERNARY_OP:
      parentsOfNode[((TernaryOp*)node2)->leftOperand].erase(node1);
      parentsOfNode[((TernaryOp*)node2)->middleOperand].erase(node1);
      parentsOfNode[((TernaryOp*)node2)->rightOperand].erase(node1);
      break;
  }

  // Remove `node1` from parentsOfNode
  parentsOfNode.erase(node1);

  return node2;
}

void Graph::concretizeErrorComponents() {
  int totalNodesInBwdDerivatives = errorAnalyzer->BwdDerivatives.size();
  int processedNodes = 0;

  // Iterate through the bwdDerivative map
  for(auto &node_bwd_derivatives : errorAnalyzer->BwdDerivatives) {
    Node* node = node_bwd_derivatives.first;

    node_bwd_derivatives.second.size();

    if(debugLevel > 2) {
      std::cout << "Processing Node " << node->id << ". Processed "
                << processedNodes << "/" << totalNodesInBwdDerivatives << std::endl;
    }

    // Iterate through the nodeBwdDerivatives map
    for(auto &node_output_bwd_derivative : node_bwd_derivatives.second) {
      Node *output_node = node_output_bwd_derivative.first;
      OptResult max_bwd = ibexInterface->FindAbsMax(*node_output_bwd_derivative.second);
      OptResult max_local_err = ibexInterface->FindAbsMax(
              const_cast<ibex::ExprNode &>(product(node->getAbsoluteError(), node->getRounding())));

      errorAnalyzer->BwdDerivatives[node][output_node] =
              (ibex::ExprNode *) &ibex::ExprConstant::new_scalar((-max_bwd.result).mag());
      node->setAbsoluteError((ibex::ExprNode *) &ibex::ExprConstant::new_scalar((-max_local_err.result).mag()));
    }

    processedNodes++;
  }
}

// Uses ibex eval to evaluate the backward derivatives and local errors and stores them in a map of node to ibex::IntervalVector, ibex::IntervalVector
void Graph::examineBwdDerivativeAndLocalError() {
  // Create a map similar to errorAnalyzer->BwdDerivatives but with a pair of double as the value
  std::map<Node *, std::map<Node *, std::pair<double, double>> > evaluatedBwdDerivatives;

  // Iterate through the bwdDerivative map
  for(auto &node_bwd_derivatives : errorAnalyzer->BwdDerivatives) {
    Node* node = node_bwd_derivatives.first;
    // Iterate through the nodeBwdDerivatives map
    for(auto &node_output_bwd_derivative : node_bwd_derivatives.second) {
      Node *output_node = node_output_bwd_derivative.first;
      OptResult max_bwd = ibexInterface->FindAbsMax(*node_output_bwd_derivative.second);
      OptResult max_local_err = ibexInterface->FindAbsMax(
              const_cast<ibex::ExprNode &>(product(node->getAbsoluteError(), node->getRounding())));

      evaluatedBwdDerivatives[node][output_node] = std::make_pair((-max_bwd.result).mag(), (-max_local_err.result).mag());
    }
  }

  // print the evaluatedBwdDerivatives
  for(auto &node_bwd_derivatives : evaluatedBwdDerivatives) {
    Node* node = node_bwd_derivatives.first;
    for(auto &node_output_bwd_derivative : node_bwd_derivatives.second) {
      Node *output_node = node_output_bwd_derivative.first;
      std::pair<double, double> bwd_local_err = node_output_bwd_derivative.second;
      std::cout << "(Output Id, Depth): " << output_node->id << ", " << output_node->depth
      << " (Node Id, Depth): " << node->id << ", " << node->depth
      << " Bwd: " << bwd_local_err.first << " Local Error: " << bwd_local_err.second << std::endl;
    }
  }

  // Store the evaluatedBwdDerivatives in a file
  std::ofstream bwd_derivatives_file("bwd_derivatives.csv");
  // Output format: Node Id, Depth, Bwd, Local Error
  bwd_derivatives_file << "Node Id,Depth,Bwd,Local Error" << std::endl;
  for(auto &node_bwd_derivatives : evaluatedBwdDerivatives) {
    Node* node = node_bwd_derivatives.first;
    for(auto &node_output_bwd_derivative : node_bwd_derivatives.second) {
      Node *output_node = node_output_bwd_derivative.first;
      std::pair<double, double> bwd_local_err = node_output_bwd_derivative.second;
      bwd_derivatives_file << node->id << "," << node->depth << ","
                            << bwd_local_err.first << ","  << bwd_local_err.second << std::endl;
    }
  }
  bwd_derivatives_file.close();
}

bool Graph::compareDAGs(ibex::ExprNode expr1, ibex::ExprNode expr2) {
  if (expr1.type_id() == expr2.type_id()) {
    switch (expr1.type_id()) {
      case ibex::ExprNode::NumExprSymbol:
        return true;
      case ibex::ExprNode::NumExprConstant:
        return expr1 == expr2;
      case ibex::ExprNode::NumExprAdd:
        return compareDAGs(((ibex::ExprAdd*)&expr1)->left, ((ibex::ExprAdd*)&expr2)->left) &&
               compareDAGs(((ibex::ExprAdd*)&expr1)->right, ((ibex::ExprAdd*)&expr2)->right);
      case ibex::ExprNode::NumExprMul:
        return compareDAGs(((ibex::ExprMul*)&expr1)->left, ((ibex::ExprMul*)&expr2)->left) &&
               compareDAGs(((ibex::ExprMul*)&expr1)->right, ((ibex::ExprMul*)&expr2)->right);
      case ibex::ExprNode::NumExprSub:
        return compareDAGs(((ibex::ExprSub*)&expr1)->left, ((ibex::ExprSub*)&expr2)->left) &&
               compareDAGs(((ibex::ExprSub*)&expr1)->right, ((ibex::ExprSub*)&expr2)->right);
      case ibex::ExprNode::NumExprDiv:
        return compareDAGs(((ibex::ExprDiv*)&expr1)->left, ((ibex::ExprDiv*)&expr2)->left) &&
               compareDAGs(((ibex::ExprDiv*)&expr1)->right, ((ibex::ExprDiv*)&expr2)->right);
      case ibex::ExprNode::NumExprSin:
        return compareDAGs(((ibex::ExprSin*)&expr1)->expr, ((ibex::ExprSin*)&expr2)->expr);
      case ibex::ExprNode::NumExprCos:
        return compareDAGs(((ibex::ExprCos*)&expr1)->expr, ((ibex::ExprCos*)&expr2)->expr);
      case ibex::ExprNode::NumExprTan:
        return compareDAGs(((ibex::ExprTan*)&expr1)->expr, ((ibex::ExprTan*)&expr2)->expr);
      case ibex::ExprNode::NumExprSinh:
        return compareDAGs(((ibex::ExprSinh*)&expr1)->expr, ((ibex::ExprSinh*)&expr2)->expr);
      case ibex::ExprNode::NumExprCosh:
        return compareDAGs(((ibex::ExprCosh*)&expr1)->expr, ((ibex::ExprCosh*)&expr2)->expr);
      case ibex::ExprNode::NumExprTanh:
        return compareDAGs(((ibex::ExprTanh*)&expr1)->expr, ((ibex::ExprTanh*)&expr2)->expr);
      case ibex::ExprNode::NumExprAsin:
        return compareDAGs(((ibex::ExprAsin*)&expr1)->expr, ((ibex::ExprAsin*)&expr2)->expr);
      case ibex::ExprNode::NumExprAcos:
        return compareDAGs(((ibex::ExprAcos*)&expr1)->expr, ((ibex::ExprAcos*)&expr2)->expr);
      case ibex::ExprNode::NumExprAtan:
        return compareDAGs(((ibex::ExprAtan*)&expr1)->expr, ((ibex::ExprAtan*)&expr2)->expr);
      case ibex::ExprNode::NumExprLog:
        return compareDAGs(((ibex::ExprLog*)&expr1)->expr, ((ibex::ExprLog*)&expr2)->expr);
      case ibex::ExprNode::NumExprSqrt:
        return compareDAGs(((ibex::ExprSqrt*)&expr1)->expr, ((ibex::ExprSqrt*)&expr2)->expr);
      case ibex::ExprNode::NumExprExp:
        return compareDAGs(((ibex::ExprExp*)&expr1)->expr, ((ibex::ExprExp*)&expr2)->expr);
      default:
        std::cout << "Unknown node type" << std::endl;
        exit(1);
    }
  }

  return false;
}

/*
 * Flattens subDAGs within min_depth and max_depth
 *
 * @param node Node to flatten subDAGs for
 * @param min_depth Lower bound of depth window to get nodes from
 * @param max_depth Upper bound of depth window to get nodes from
 *
 * @return A set of nodes that are flattened subDAGs
 */
std::set<Node *> Graph::FlattenSubDAGS(Node *node, unsigned int min_depth, unsigned int max_depth) {
  assert(min_depth <= max_depth && "Invalid bounds for flattening");

  std::set<Node *> nodes_to_flatten;

  // Flatten nodes from children of node within depth windowI
  switch (node->type) {
    case NodeType::INTEGER:
      break;
    case NodeType::FLOAT:
      break;
    case NodeType::DOUBLE:
      break;
    case NodeType::FREE_VARIABLE:
      break;
    case NodeType::VARIABLE:
      break;
    case NodeType::UNARY_OP:
      if (((UnaryOp*)node)->Operand->depth >= min_depth && ((UnaryOp*)node)->Operand->depth <= max_depth) {
        nodes_to_flatten.insert(((UnaryOp *)node)->Operand);
      }
      if (((UnaryOp*)node)->Operand->depth > min_depth) {
        std::set_union(nodes_to_flatten.begin(), nodes_to_flatten.end(),
                       FlattenSubDAGS(((UnaryOp*)node)->Operand, min_depth, max_depth).begin(),
                       FlattenSubDAGS(((UnaryOp*)node)->Operand, min_depth, max_depth).end(),
                       std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
      }
      break;
    case NodeType::BINARY_OP:
      if (((BinaryOp*)node)->leftOperand->depth >= min_depth && ((BinaryOp*)node)->leftOperand->depth <= max_depth) {
        nodes_to_flatten.insert(((BinaryOp *)node)->leftOperand);
      }
      if (((BinaryOp*)node)->rightOperand->depth >= min_depth && ((BinaryOp*)node)->rightOperand->depth <= max_depth) {
        nodes_to_flatten.insert(((BinaryOp *)node)->rightOperand);
      }
      if (((BinaryOp*)node)->leftOperand->depth > min_depth) {
        std::set_union(nodes_to_flatten.begin(), nodes_to_flatten.end(),
                       FlattenSubDAGS(((BinaryOp*)node)->leftOperand, min_depth, max_depth).begin(),
                       FlattenSubDAGS(((BinaryOp*)node)->leftOperand, min_depth, max_depth).end(),
                       std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
      }
      if (((BinaryOp*)node)->rightOperand->depth > min_depth) {
        std::set_union(nodes_to_flatten.begin(), nodes_to_flatten.end(),
                       FlattenSubDAGS(((BinaryOp*)node)->rightOperand, min_depth, max_depth).begin(),
                       FlattenSubDAGS(((BinaryOp*)node)->rightOperand, min_depth, max_depth).end(),
                       std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
      }
      break;
    case NodeType::TERNARY_OP:
      if (((TernaryOp*)node)->leftOperand->depth >= min_depth && ((TernaryOp*)node)->leftOperand->depth <= max_depth) {
        nodes_to_flatten.insert(((TernaryOp *)node)->leftOperand);
      }
      if (((TernaryOp*)node)->middleOperand->depth >= min_depth && ((TernaryOp*)node)->middleOperand->depth <= max_depth) {
        nodes_to_flatten.insert(((TernaryOp *)node)->middleOperand);
      }
      if (((TernaryOp*)node)->rightOperand->depth >= min_depth && ((TernaryOp*)node)->rightOperand->depth <= max_depth) {
        nodes_to_flatten.insert(((TernaryOp *)node)->rightOperand);
      }
      if (((TernaryOp*)node)->leftOperand->depth > min_depth) {
        std::set_union(nodes_to_flatten.begin(), nodes_to_flatten.end(),
                       FlattenSubDAGS(((TernaryOp*)node)->leftOperand, min_depth, max_depth).begin(),
                       FlattenSubDAGS(((TernaryOp*)node)->leftOperand, min_depth, max_depth).end(),
                       std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
      }
      if (((TernaryOp*)node)->middleOperand->depth > min_depth) {
        std::set_union(nodes_to_flatten.begin(), nodes_to_flatten.end(),
                       FlattenSubDAGS(((TernaryOp*)node)->middleOperand, min_depth, max_depth).begin(),
                       FlattenSubDAGS(((TernaryOp*)node)->middleOperand, min_depth, max_depth).end(),
                       std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
      }
      if (((TernaryOp*)node)->rightOperand->depth > min_depth) {
        std::set_union(nodes_to_flatten.begin(), nodes_to_flatten.end(),
                       FlattenSubDAGS(((TernaryOp*)node)->rightOperand, min_depth, max_depth).begin(),
                       FlattenSubDAGS(((TernaryOp*)node)->rightOperand, min_depth, max_depth).end(),
                       std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
      }
      break;
    default:
      std::cout << "Unknown node type" << std::endl;
      exit(1);
  }

  return {};
}

/*
 * Finds common nodes within min_depth and max_depth from the flattened children subDAGs
 *
 * @param nodes Set of nodes to find common nodes from
 * @param min_depth Lower bound of depth window to get nodes from
 * @param max_depth Upper bound of depth window to get nodes from
 *
 * @return A set of nodes that are common to all nodes in node's children
 */
std::set<Node *> Graph::FindCommonNodes(Node * node, unsigned int min_depth, unsigned int max_depth) {
  std::set<Node *> common_nodes;

  // Create a list of flattened subDAGs of children of node
  std::vector<std::set<Node *>> flattened_subDAGs;
  switch (node->type) {
    case NodeType::INTEGER:
      break;
    case NodeType::FLOAT:
      break;
    case NodeType::DOUBLE:
      break;
    case NodeType::FREE_VARIABLE:
      break;
    case NodeType::VARIABLE:
      break;
    case NodeType::UNARY_OP:
      flattened_subDAGs.push_back(FlattenSubDAGS(((UnaryOp*)node)->Operand, min_depth, max_depth));
      break;
    case NodeType::BINARY_OP:
      flattened_subDAGs.push_back(FlattenSubDAGS(((BinaryOp*)node)->leftOperand, min_depth, max_depth));
      flattened_subDAGs.push_back(FlattenSubDAGS(((BinaryOp*)node)->rightOperand, min_depth, max_depth));
      break;
    case NodeType::TERNARY_OP:
      flattened_subDAGs.push_back(FlattenSubDAGS(((TernaryOp*)node)->leftOperand, min_depth, max_depth));
      flattened_subDAGs.push_back(FlattenSubDAGS(((TernaryOp*)node)->middleOperand, min_depth, max_depth));
      flattened_subDAGs.push_back(FlattenSubDAGS(((TernaryOp*)node)->rightOperand, min_depth, max_depth));
      break;
    default:
      std::cout << "Unknown node type" << std::endl;
      exit(1);
  }

  flattened_subDAGs.push_back(std::set<Node *>({node}));

//  std::cout << "Flattened SubDAGs:" << std::endl;
//  for (auto &flattened_subDAG : flattened_subDAGs) {
//    for (auto &node : flattened_subDAG) {
//      std::cout << "\t" << *node << std::endl;
//    }
//  }

  // Find common nodes
  for (auto &flattened_subDAG : flattened_subDAGs) {
    if (common_nodes.empty()) {
      common_nodes = flattened_subDAG;
    }
    else {
      std::set<Node *> temp;
      std::set_intersection(common_nodes.begin(), common_nodes.end(),
                            flattened_subDAG.begin(), flattened_subDAG.end(),
                            std::inserter(temp, temp.end()));
      common_nodes = temp;
    }
  }

  return common_nodes;
}

/*
 * Finds common nodes within min_depth and max_depth from the flattened children subDAGs
 *
 * @param nodes Set of nodes to find common nodes from
 * @param min_depth Lower bound of depth window to get nodes from
 * @param max_depth Upper bound of depth window to get nodes from
 *
 * @return A set of nodes that are common to all nodes in nodes
 */
std::map<Node *, std::set<Node *>>
Graph::FindCommonDependencies(std::set<Node *> nodes, unsigned int min_depth, unsigned int max_depth) {
  std::map<Node *, std::set<Node *>> common_dependencies;

  // Populate common_dependencies with common_nodes from each node's common node list
  for (auto &node : nodes) {
    std::set<Node*> initial_dependence_list = FindCommonNodes(node, min_depth, max_depth);
    std::vector<std::set<Node*>> common_nodes_list;

    // Populate common_nodes_list with common_nodes from each node's common node list
    for (auto &node : initial_dependence_list) {
      common_nodes_list.push_back(FindCommonNodes(node, min_depth, max_depth));
    }

    std::set<Node*> redundant_nodes;

    // Unionize common_nodes_list into redundant_nodes
    for (auto &common_nodes : common_nodes_list) {
      std::set_union(redundant_nodes.begin(), redundant_nodes.end(),
                     common_nodes.begin(), common_nodes.end(),
                     std::inserter(redundant_nodes, redundant_nodes.end()));
    }

    // Get the set difference between initial_dependence_list and redundant_nodes
    std::set<Node*> common_nodes;
    std::set_difference(initial_dependence_list.begin(), initial_dependence_list.end(),
                        redundant_nodes.begin(), redundant_nodes.end(),
                        std::inserter(common_nodes, common_nodes.end()));
    if (!common_nodes.empty()) {
      common_dependencies[node] = common_nodes;
    } else {
      common_dependencies[node].insert(node);
    }
  }

  return common_dependencies;
}

/*
 * Filters nodes with operation op within depth max_depth
 *
 * @param op Operation to filter nodes with
 * @param max_depth Maximum depth of the graph
 *
 * @return A vector of op operation nodes within max_depth
 */
std::set<Node *> Graph::FilterNodesWithOperationWithinDepth(Node::Op op, unsigned int max_depth) {
  std::set<Node *> nodes_with_op;

  std::copy_if(nodes.begin(), nodes.end(), std::inserter(nodes_with_op, nodes_with_op.end()),
               [op, max_depth](Node *node) {
                 return node->depth <= max_depth && (node->isUnaryOp() && ((UnaryOp *) node)->op == op) ||
                        (node->isBinaryOp() && ((BinaryOp *) node)->op == op);
               });

  return nodes_with_op;
}



/*
 * Filters nodes with depth between lower_bound and upper_bound
 *
 * @param max_depth Maximum depth of the graph
 * @param lower_bound Lower bound of the abstraction window
 * @param upper_bound Upper bound of the abstraction window
 *
 * @return A vector of nodes that are candidates for abstraction
 */
std::set<Node *>
Graph::FilterCandidatesForAbstraction(unsigned int max_depth, unsigned int lower_bound, unsigned int upper_bound) {
  assert(lower_bound <= upper_bound && upper_bound <= max_depth && "Invalid bounds for abstraction");

  std::set<Node *> nodes_with_op = FilterNodesWithOperationWithinDepth(Node::Op::DIV, max_depth);

  // Print nodes with op
  if (debugLevel > 4) {
    std::cout << "Nodes with op:" << std::endl;
    for (auto &node: nodes_with_op) {
      std::cout << "\t" << *node << std::endl;
    }
  }
  if (logLevel > 4) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Nodes with op:" << std::endl;
    for (auto &node: nodes_with_op) {
      log.logFile << "\t" << *node << std::endl;
    }
  }

  std::map<Node *, std::set<Node *>> common_dependencies = FindCommonDependencies(nodes_with_op, lower_bound, upper_bound);

  // Print common dependencies
  if (debugLevel > 4) {
    std::cout << "Common dependencies:" << std::endl;
    for (auto &common_dependency : common_dependencies) {
      std::cout << "\t" << *common_dependency.first << " : ";
      for (auto &node : common_dependency.second) {
        std::cout << *node << " ";
      }
      std::cout << std::endl;
    }
  }
  if (logLevel > 4) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Common dependencies:" << std::endl;
    for (auto &common_dependency : common_dependencies) {
      log.logFile << "\t" << *common_dependency.first << " : ";
      for (auto &node : common_dependency.second) {
        log.logFile << *node << " ";
      }
      log.logFile << std::endl;
    }
  }

  // Unionize the node set from common_dependencies
  std::set<Node *> common_dependencies_set;
  for (auto &common_dependency : common_dependencies) {
    std::set_union(common_dependencies_set.begin(), common_dependencies_set.end(),
                   common_dependency.second.begin(), common_dependency.second.end(),
                   std::inserter(common_dependencies_set, common_dependencies_set.end()));
  }

  if (common_dependencies_set.empty()) {
    if (debugLevel > 4) {
      std::cout << "Empty dependence set! Generating candidates!" << std::endl;
    }
    if (logLevel > 4) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "Empty dependence set! Generating candidates!" << std::endl;
    }

    // Get all nodes from depthTable within the depth window with node type UnaryOp, BinaryOp, or TernaryOp
    for (auto &depth_table : depthTable) {
      if (depth_table.first >= lower_bound && depth_table.first <= upper_bound) {
        std::set_union(common_dependencies_set.begin(), common_dependencies_set.end(),
                       depth_table.second.begin(), depth_table.second.end(),
                       std::inserter(common_dependencies_set, common_dependencies_set.end()));
      }
    }
  } else {
    unsigned int local_max_depth=-1;
    // Get the greatest depth from nodes in common_dependencies_set
    for (auto &node : common_dependencies_set) {
      if (node->depth > local_max_depth) {
        local_max_depth = node->depth;
      }
    }

    // Get nodes from common_dependencies_set with depth equal to local_max_depth
    std::set<Node *> common_dependencies_set;
    for (auto &node : common_dependencies_set) {
      if (node->depth == local_max_depth) {
        common_dependencies_set.insert(node);
      }
    }
  }

  // Print common dependencies set
  if (debugLevel > 4) {
    std::cout << "Common dependencies set:" << std::endl;
    for (auto &node : common_dependencies_set) {
      std::cout << "\t" << *node << std::endl;
    }
  }
  if (logLevel > 4) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Common dependencies set:" << std::endl;
    for (auto &node : common_dependencies_set) {
      log.logFile << "\t" << *node << std::endl;
    }
  }

  return common_dependencies_set;
}

std::pair<unsigned int, std::set<Node*>> Graph::selectNodesForAbstraction(unsigned int max_depth,
                                                    unsigned int bound_min_depth,
                                                    unsigned int bound_max_depth) {
  assert(bound_min_depth <= bound_max_depth && bound_max_depth <= max_depth && "Invalid bounds for abstraction");
  std::set<Node*> nodes_to_abstract;

  if (debugLevel > 2) {
    std::cout << "Selecting nodes for abstraction..." << std::endl;
  }
  if (logLevel > 2) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Selecting nodes for abstraction..." << std::endl;
  }

  // Abstraction window is just 1 level wide
  if (bound_min_depth == bound_max_depth && bound_max_depth <= max_depth) {
    return std::make_pair(bound_min_depth, depthTable[bound_min_depth]);
  }

  std::set<Node*> initialCandidateList = FilterCandidatesForAbstraction(max_depth, bound_min_depth, bound_max_depth);

  unsigned int local_max_depth = bound_max_depth;


  // Keep increasing local_max_depth until initialCandidateList is not empty
  while (initialCandidateList.empty() && local_max_depth <= max_depth) {
    local_max_depth+=5;
    initialCandidateList = FilterCandidatesForAbstraction(max_depth, bound_min_depth, local_max_depth);
  }

  if (initialCandidateList.empty()) {
    if (debugLevel > 2) {
      std::cout << "No candidates found!" << std::endl;
    }
    if (logLevel > 2) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "No candidates found!" << std::endl;
    }
    return std::make_pair(-1, std::set<Node*>());
  } else {
    local_max_depth = 0;
    // Set local_max_depth to the greatest depth of nodes in initialCandidateList
    for (auto &node : initialCandidateList) {
      if (node->depth > local_max_depth) {
        local_max_depth = node->depth;
      }
    }

    auto f = [&local_max_depth](Node *x) {
      return float(x->depth)/(local_max_depth + 0.01);
    };

    auto g = [](Node *x, auto y) {
      return (-1)*y* log2(y)*x->parents.size();
    };

    // Create a list of cost
    std::map<Node*, double> cost_dict;

    // Compute g(x, f(x)) for each node in initialCandidateList
    for (auto &node : initialCandidateList) {
      cost_dict[node] = g(node, f(node));
    }

    // Sum cost of all nodes with same depth
    std::map<int, double> cost_sum_dict;
    for (auto &node : initialCandidateList) {
      cost_sum_dict[node->depth] += cost_dict[node];
    }

    // Print cost_sum_dict
    if (debugLevel > 3) {
      std::cout << "Cost Sum Dict:" << std::endl;
      for (auto &cost_sum : cost_sum_dict) {
        std::cout << "\t" << cost_sum.first << " : " << cost_sum.second << std::endl;
      }
    }
    if (logLevel > 3) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "Cost Sum Dict:" << std::endl;
      for (auto &cost_sum : cost_sum_dict) {
        log.logFile << "\t" << cost_sum.first << " : " << cost_sum.second << std::endl;
      }
    }

    // Get the depth with the greatest cost
    int abstraction_depth = -1;
    double greatest_cost = -1;
    for (auto &cost_sum : cost_sum_dict) {
      if (cost_sum.second > greatest_cost) {
        greatest_cost = cost_sum.second;
        abstraction_depth = cost_sum.first;
      }
    }

    // Get nodes with depth equal to depth_with_greatest_cost
    auto candidate_nodes = depthTable[abstraction_depth];

    // Print max depth and abstraction depth
    if (debugLevel > 2) {
      std::cout << "Max Depth: " << max_depth << std::endl;
      std::cout << "Abstraction Depth: " << abstraction_depth << std::endl;
    }
    if (logLevel > 2) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "Max Depth: " << max_depth << std::endl;
      log.logFile << "Abstraction Depth: " << abstraction_depth << std::endl;
    }

    return std::make_pair(abstraction_depth, candidate_nodes);
  }
}


void Graph::performAbstraction(unsigned int bound_min_depth, unsigned int bound_max_depth) {
  // Get max depth using keys in depthTable
  unsigned int max_depth = depthTable.rbegin()->first;

  unsigned int abstraction_count = 1;
  if (debugLevel > 0) {
    std::cout << "Performing abstraction with window [" << bound_min_depth << ", " << bound_max_depth << "]" << std::endl;
  }
  if (logLevel > 0) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Performing abstraction with window [" << bound_min_depth << ", " << bound_max_depth << "]" << std::endl;
  }

  while (max_depth >= bound_max_depth && max_depth >= bound_min_depth) {
    auto [abstraction_depth, candidate_nodes] = selectNodesForAbstraction(max_depth, bound_min_depth, bound_max_depth);

    if (debugLevel > 1) {
      std::cout << "Abstraction count: " << abstraction_count << std::endl;

      if (debugLevel > 4 && !candidate_nodes.empty()) {
        // Print candidate nodes
        std::cout << "Candidate Nodes:" << std::endl;
        for (auto &node : candidate_nodes) {
          std::cout << "\t" << *node << std::endl;
        }
      }
    }
    if (logLevel > 1) {
      log.logFile << "Abstraction count: " << abstraction_count << std::endl;

      if (logLevel > 3 && !candidate_nodes.empty()) {
        // Print candidate nodes
        log.logFile << "Candidate Nodes:" << std::endl;
        for (auto &node : candidate_nodes) {
          log.logFile << "\t" << *node << std::endl;
        }
      }
    }

    if (!candidate_nodes.empty()) {
      abstraction_count++;
      abstractionMetrics[abstraction_count]["bound_min"] = bound_min_depth;
      abstractionMetrics[abstraction_count]["bound_max"] = bound_max_depth;
      abstractionMetrics[abstraction_count]["abstraction_depth"] = abstraction_depth;
      abstractionMetrics[abstraction_count]["num_candidate_nodes"] = candidate_nodes.size();

      // Modify the AST
      SimplifyWithAbstraction(candidate_nodes, max_depth);

      max_depth = depthTable.rbegin()->first;

      // The expressions have been built to this point, so we can query the IBEX expression for op counts
      unsigned int max_operators_count = 1000;
      for(auto &node : candidate_nodes) {
        unsigned int op_count = node->getExprNode()->size;
        if (op_count < max_operators_count) {
          max_operators_count = op_count;
        }
      }

      // These metrics are computed after the abstraction is performed
      abstractionMetrics[abstraction_count]["max_operators_count"] = max_operators_count;
      abstractionMetrics[abstraction_count]["max_depth"] = max_depth;

      if (max_operators_count < 1000 && max_depth > 8 && bound_min_depth > 5) {
        if(bound_max_depth > max_depth) {
          bound_max_depth = max_depth;
        } else if(bound_max_depth - bound_min_depth > 4) {
          bound_max_depth = bound_max_depth - 2;
        }

        if(bound_max_depth - bound_min_depth > 4) {
          bound_min_depth = bound_min_depth - 2;
        }
      } else if (max_depth <= bound_max_depth && max_depth > bound_min_depth) {
        bound_max_depth = max_depth;
        assert(bound_max_depth >= bound_min_depth);
      }
    } else {
      if (debugLevel > 2) {
        std::cout << "No candidates found!" << std::endl;
      }
      if (logLevel > 2) {
        assert(log.logFile.is_open() && "Log file not open");
        log.logFile << "No candidates found!" << std::endl;
      }
    }
  }

  if (debugLevel > 0) {
    std::cout << "Abstraction complete!" << std::endl;
  }
  if (logLevel > 0) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Abstraction complete!" << std::endl;
  }
}

void Graph::FindOutputExtrema(const std::set<Node *>& candidate_nodes) {
  if(debugLevel > 1) {
    std::cout << "Finding output extremas...";
    if(debugLevel > 2) {
      std::cout << " for " << candidate_nodes.size() << " nodes" << std::endl;
    }
    std::cout << std::endl;
  }
  if(logLevel > 1) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Finding output extremas...";
    if(logLevel > 2) {
      log.logFile << " for " << candidate_nodes.size() << " nodes" << std::endl;
    }
    log.logFile << std::endl;
  }

//  if(debugLevel > 4) {
//    for (auto &node: candidate_nodes) {
//      std::cout << ibexInterface->dumpFunction(node->getExprNode()) << std::endl;
//    }
//  }
//  if(logLevel > 4) {
//    for (auto &node: candidate_nodes) {
//      assert(log.logFile.is_open() && "Log file not open");
//      log.logFile << ibexInterface->dumpFunction(node->getExprNode()) << std::endl;
//    }
//  }

  std::map<Node *, OptResult> max;
  for (auto &node : candidate_nodes) {
    if(debugLevel > 3) {
      std::cout << "Finding max for: " << node->id << std::endl;
      if(debugLevel > 4) {
        std::cout << "Output Expression: " << *node->getExprNode() << std::endl;
      }
    }
    if(logLevel > 3) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "Finding max for: " << node->id << std::endl;
      if(logLevel > 4) {
        log.logFile << "Output Expression: " << *node->getExprNode() << std::endl;
      }
    }
    max[node] = ibexInterface->FindAbsMax(*node->getExprNode());

    // print the output interval - Max have to be flipped since we find the min of the negative of the function
    if(debugLevel > 3) {
      std::cout << "Max Interval: " << -max[node].result << std::endl;
    }
    if(logLevel > 3) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "Max Interval: " << -max[node].result << std::endl;
    }
  }

  for (auto &node : candidate_nodes) {
    if (debugLevel > 4) {
      std::cout << "Output Extrema for: " << *node->getExprNode() << std::endl;
    }
    if (logLevel > 4) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "Output Extrema for: " << *node->getExprNode() << std::endl;
    }
    errorAnalysisResults[node].outputExtrema = -max[node].result;
    errorAnalysisResults[node].numOptimizationCalls += 1 + errorAnalyzer->nodeNumOptCallsMap[node];

    if(debugLevel > 4) {
      std::cout << "Output Extrema: " << errorAnalysisResults[node].outputExtrema << std::endl;
    }
    if(logLevel > 4) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "Output Extrema: " << errorAnalysisResults[node].outputExtrema << std::endl;
    }
  }

  if (debugLevel > 1) {
    std::cout << "Output extremas found!" << std::endl;
  }
  if (logLevel > 1) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Output extremas found!" << std::endl;
  }
}

void Graph::FindErrorExtrema(const std::set<Node *>& candidate_nodes) {
  if (debugLevel > 1) {
    std::cout << "Finding error extrema...";
    if(debugLevel > 2) {
      std::cout << " for " << candidate_nodes.size() << " nodes" << std::endl;
    }
    std::cout << std::endl;
  }
  if(logLevel > 1) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Finding error extremas...";
    if(logLevel > 2) {
      log.logFile << " for " << candidate_nodes.size() << " nodes" << std::endl;
    }
    log.logFile << std::endl;
  }

  setupDerivativeComputation(candidate_nodes);

  errorAnalyzer->derivativeComputingDriver();

  if (concretize_error_components) {
    concretizeErrorComponents();
  }

  errorAnalyzer->errorComputingDriver(candidate_nodes, ibexInterface);

  if (collect_error_component_data) {
    examineBwdDerivativeAndLocalError();
  }

//  if(!validationFile.empty()) {
//    std::cout << "Comparing Error expression with validation file: " << validationFile << std::endl;
//    try {
//      ibex::Function f = ibexInterface->parseIbexFunctionFromFile(validationFile.c_str());
//      std::cout << "Function: " << f << std::endl;
////      compareDAGs(f.expr(), ibexInterface->getSystem()->goal->expr());
//    } catch(ibex::SyntaxError& e) {
//      std::cout << e << std::endl;
//    }
//  }

  std::map<Node *, OptResult> max;
  for (auto &node : candidate_nodes) {
    if (debugLevel > 3) {
      std::cout << "Finding max for: " << node->id << std::endl;
      if (debugLevel > 4) {
        std::cout << "Error Expression: " << *errorAnalyzer->ErrAccumulator[node] << std::endl;
      }
    }
    if (logLevel > 3) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "Finding max for: " << node->id << std::endl;
      if (logLevel > 4) {
        log.logFile << "Error Expression: " << *errorAnalyzer->ErrAccumulator[node] << std::endl;
      }
    }
    max[node] = ibexInterface->FindAbsMax(*errorAnalyzer->ErrAccumulator[node]);

    // print the error interval - Max have to be flipped since we find the min of the negative of the function
    if (debugLevel > 3) {
      std::cout << "Max Interval: " << -max[node].result << std::endl;
    }
    if (logLevel > 3) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "Max Interval: " << -max[node].result << std::endl;
    }
  }

  for (auto &node : candidate_nodes) {
    if (debugLevel > 4) {
      std::cout << "Error Extrema for: " << *errorAnalyzer->ErrAccumulator[node] << std::endl;
    }
    if (logLevel > 4) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "Error Extrema for: " << *errorAnalyzer->ErrAccumulator[node] << std::endl;
    }
    errorAnalysisResults[node].errorExtrema = ibex::Interval(-(max[node].result) * pow(2, -53));

    errorAnalysisResults[node].OptPoint = max[node].optimumPoint;
    errorAnalysisResults[node].totalOptimizationTime += max[node].optimizationTime;
    errorAnalysisResults[node].numOptimizationCalls += 1;

    if (debugLevel > 4) {
      std::cout << "Error Extrema: " << errorAnalysisResults[node].errorExtrema
                << " at " << errorAnalysisResults[node].OptPoint << std::endl;
    }
    if (logLevel > 4) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "Error Extrema: " << errorAnalysisResults[node].errorExtrema
                  << " at " << errorAnalysisResults[node].OptPoint << std::endl;
    }
  }

  if (debugLevel > 1) {
    std::cout << "Error extremas found!" << std::endl;
  }
  if (logLevel > 1) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Error extremas found!" << std::endl;
  }
}

std::map<Node *, ErrorAnalysisResult> Graph::SimplifyWithAbstraction(const std::set<Node *>& candidate_nodes, unsigned int max_depth, bool isFinal) {
  if (debugLevel > 0 && isFinal) {
    std::cout << "Final computation..." << std::endl;
  }
  if (logLevel > 0 && isFinal) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Final computation..." << std::endl;
  }

  ibexInterface->setInputIntervals(inputs);
  generateIbexSymbols();
  ibexInterface->setVariables(inputs, symbolTables[currentScope]->table);
  generateExprDriver(candidate_nodes);

  FindErrorExtrema(candidate_nodes);
  FindOutputExtrema(candidate_nodes);

  if(isFinal) {
    if (debugLevel > 0) {
      std::cout << "Final Computation complete!" << std::endl;
    }
    if (logLevel > 0) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "Final Computation complete!" << std::endl;
    }
    return errorAnalysisResults;
  }

  if (debugLevel > 1) {
    std::cout << "Abstracting nodes ";
    for (auto &node : candidate_nodes) {
      std::cout << node->id << ", ";
    }
    std::cout << std::endl;
  }

  if (logLevel > 1) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Abstracting nodes ";
    for (auto &node : candidate_nodes) {
      log.logFile << node->id << ", ";
    }
    log.logFile << std::endl;
  }

  std::map<Node *, std::vector<ibex::Interval>> results;

  for (auto &node : candidate_nodes) {
    results[node].push_back(errorAnalysisResults[node].outputExtrema);
    results[node].push_back(errorAnalysisResults[node].errorExtrema * pow(2, +53));
  }

  AbstractNodes(results);
  RebuildAST();

  return errorAnalysisResults;
}

/*
 * Modifies the probe list to include nodes that are common to all nodes in the probe list
 *
 * @param probeList List of nodes to modify
 *
 * @return A list of nodes that are common to all nodes in the probe list
 */
std::vector<Node *> Graph::ModProbeList() {
  std::vector<Node*> probe_list;

  // Get nodes from symbol table corresponding to the output variables
  for (auto &output : outputs) {
    probe_list.push_back(symbolTables[currentScope]->table[output]);
  }

  return probe_list;
}

/*
 * Abstracts nodes in results
 *
 * @param results A map of nodes to their corresponding intervals
 */
void Graph::AbstractNodes(std::map<Node *, std::vector<ibex::Interval>> results) {
  if (debugLevel > 1) {
    std::cout << "Abstracting nodes..." << std::endl;
  }
  if (logLevel > 1) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Abstracting nodes..." << std::endl;
  }

  // Turn node in results into VariableNodes and create corresponding FreeVariable nodes
  for (auto &singleResult : results) {
    auto node = singleResult.first;

    VariableNode *converted_node;

    // Convert node to VariableNode
    converted_node = new VariableNode(*node);
    converted_node->setAbsoluteError(&ibex::ExprConstant::new_scalar(singleResult.second[1].ub()));

    // Add converted node to nodes and symbol table
    nodes.insert(converted_node);
    symbolTables[currentScope]->table[converted_node->variable->name] = converted_node;

    // Create corresponding FreeVariable node using the singleResult IntervalVector
    auto *free_node = new FreeVariable(*new ibex::Interval(singleResult.second[0]), Node::RoundingType::FL64);
    inputs[converted_node->variable->name] = free_node;

    // Add free node to nodes and inputs
    nodes.insert(free_node);
    free_node->setAbsoluteError(&ibex::ExprConstant::new_scalar(singleResult.second[1].ub()));
    free_node->setRounding(converted_node->getRounding());

    if (debugLevel > 1) {
      std::cout << "Converted Node " << node->id << " --> " << *converted_node->variable << " (" << converted_node->id << ")" << std::endl;
      if (debugLevel > 2) {
        std::cout << *singleResult.first << " : "
                  << "\n\tOutput: " << singleResult.second[0] << ","
                  << "\n\tError: " << singleResult.second[1] << ","
                  << "\n\tNumber of Optimizer Calls: " << errorAnalysisResults[node].numOptimizationCalls << std::endl;
      }
    }
    if (logLevel > 1) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "Converted Node " << node->id << " --> " << *converted_node->variable << " (" << converted_node->id << ")" << std::endl;
      if (logLevel > 2) {
        log.logFile << *singleResult.first << " : "
                    << "\n\tOutput: " << singleResult.second[0] << ","
                    << "\n\tError: " << singleResult.second[1] << ","
                    << "\n\tNumber of Optimizer Calls: " << errorAnalysisResults[node].numOptimizationCalls << std::endl;
      }
    }
  }

  if (debugLevel > 1) {
    std::cout << "Nodes abstracted!" << std::endl;
  }
  if (logLevel > 1) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Nodes abstracted!" << std::endl;
  }
}

/*
 * Rebuilds the AST post abstraction
 */
void Graph::RebuildAST() {
  if (debugLevel > 1) {
    std::cout << "Rebuilding AST..." << std::endl;
  }
  if (logLevel > 1) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Rebuilding AST..." << std::endl;
  }

  std::vector<Node*> probe_list = ModProbeList();

  std::map<Node*, unsigned int> completed;

  // Recursively call RebuildASTNode on nodes in probe_list if not already completed
  for (auto &node : probe_list) {
    if (completed.find(node) == completed.end()) {
      RebuildASTNode(node, completed);
    }
  }

  // Get max depth among nodes in probe_list
  int max_depth = -1;
  for (auto &node : probe_list) {
    if (node->depth > max_depth) {
      max_depth = node->depth;
    }
  }

  // Get total number of nodes before
  unsigned int num_nodes = 0;
  for (auto &depth_table : depthTable) {
    num_nodes += depth_table.second.size();
  }

  // Print num_nodes before
  if (debugLevel > 2) {
    std::cout << "Num nodes before: " << num_nodes << std::endl;
  }
  if (logLevel > 2) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Num nodes before: " << num_nodes << std::endl;
  }

  // Modify depthTable using the completed map
  depthTable.clear();
  for (auto &node : completed) {
    depthTable[node.second].insert(node.first);
  }

  // Get total number of nodes after
  num_nodes = 0;
  for (auto &depth_table : depthTable) {
    num_nodes += depth_table.second.size();
  }

  // Print num_nodes after
  if (debugLevel > 2) {
    std::cout << "Num nodes after: " << num_nodes << std::endl;
  }
  if (logLevel > 2) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "Num nodes after: " << num_nodes << std::endl;
  }

  if (debugLevel > 1) {
    std::cout << "AST rebuilt!" << std::endl;
  }
  if (logLevel > 1) {
    assert(log.logFile.is_open() && "Log file not open");
    log.logFile << "AST rebuilt!" << std::endl;
  }
}

void Graph::RebuildASTNode(Node *node, std::map<Node *, unsigned int> &completed) {
  // Recursively call RebuildASTNode on children of node if not already completed
  switch(node->type) {
    case NodeType::INTEGER:
    case NodeType::FLOAT:
    case NodeType::DOUBLE:
    case NodeType::FREE_VARIABLE:
    case NodeType::VARIABLE:
      node->depth = 0;
      break;
    case NodeType::UNARY_OP:
      if(((UnaryOp*)node)->op != Node::Op::NEG) {
        if (completed.find(((UnaryOp*)node)->Operand) == completed.end()) {
          RebuildASTNode(((UnaryOp*)node)->Operand, completed);
        }

        node->depth = ((UnaryOp *) node)->Operand->depth + 1;
        completed[node] = node->depth;
      }
      break;
    case NodeType::BINARY_OP:
      if (completed.find(((BinaryOp*)node)->leftOperand) == completed.end()) {
        RebuildASTNode(((BinaryOp*)node)->leftOperand, completed);
      }
      if (completed.find(((BinaryOp*)node)->rightOperand) == completed.end()) {
        RebuildASTNode(((BinaryOp*)node)->rightOperand, completed);
      }
      node->depth = std::max(((BinaryOp*)node)->leftOperand->depth, ((BinaryOp*)node)->rightOperand->depth) + 1;
      completed[node] = node->depth;
      break;
    case NodeType::TERNARY_OP:
      if (completed.find(((TernaryOp*)node)->leftOperand) == completed.end()) {
        RebuildASTNode(((TernaryOp*)node)->leftOperand, completed);
      }
      if (completed.find(((TernaryOp*)node)->middleOperand) == completed.end()) {
        RebuildASTNode(((TernaryOp*)node)->middleOperand, completed);
      }
      if (completed.find(((TernaryOp*)node)->rightOperand) == completed.end()) {
        RebuildASTNode(((TernaryOp*)node)->rightOperand, completed);
      }
      node->depth = std::max(std::max(((TernaryOp*)node)->leftOperand->depth, ((TernaryOp*)node)->middleOperand->depth),
                             ((TernaryOp*)node)->rightOperand->depth) + 1;
      completed[node] = node->depth;
      break;
    default:
      std::cout << "Unknown node type" << std::endl;
      break;
  }

  // Modify node
  if ((node->isUnaryOp() && ((UnaryOp*)node)->op != Node::Op::NEG) || node->isBinaryOp() || node->isTernaryOp()) {
    completed[node] = node->depth;
  } else {
    node->depth = 0;
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
    if(debugLevel > 0) {
      std::cout << "Parsing..." << std::endl;
    }
    if(logLevel > 0) {
      assert(log.logFile.is_open() && "Log file not open");
      log.logFile << "Parsing..." << std::endl;
    }
    createNewSymbolTable();
    if(yyparse(this)) {
      std::cout << "Parsing failed" << std::endl;
      return 1;
    }
    else {
      if (debugLevel > 0) {
        std::cout << "Parsing successful!" << std::endl;
      }
      if (logLevel > 0) {
        assert(log.logFile.is_open() && "Log file not open");
        log.logFile << "Parsing successful!" << std::endl;
      }
    }
  } while (!feof(yyin));

//  std::cout << *graph << std::endl;

  return 0;
}
