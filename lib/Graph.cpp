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

void Graph::setupDerivativeComputation(std::set<Node*> nodes) {
  // Set up output
  // Assuming there is only one output
  // TODO: Change this for multiple outputs
  // Get the max depth of the nodes
  unsigned int max_depth = 0;
  for (auto &node : nodes) {
    if (node->depth > max_depth) {
      max_depth = node->depth;
    }
  }

  // Insert nodes with max depth into worklist
  for (auto &node : nodes) {
    if (node->depth == max_depth) {
      errorAnalyzer->workList.insert(node);
    }
  }

  // Set BwdDerivatives of each node with respect to itself to 1
  for (auto &node : nodes) {
    errorAnalyzer->BwdDerivatives[node][node] = (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(1);
  }

  // Set numParentsOfNode of each node to the number of parents it has
  for (auto &node : nodes) {
    errorAnalyzer->numParentsOfNode[node] = node->parents.size();
  }

//  errorAnalyzer->workList.insert(symbolTables[currentScope]->table[outputs[0]]);
//  errorAnalyzer->BwdDerivatives[symbolTables[currentScope]->table[outputs[0]]][symbolTables[currentScope]->table[outputs[0]]] =
//          (ibex::ExprNode *) &ibex::ExprConstant::new_scalar(1);
//  errorAnalyzer->numParentsOfNode[symbolTables[currentScope]->table[outputs[0]]] =
//          symbolTables[currentScope]->table[outputs[0]]->parents.size();
}

void Graph::errorComputingDriver(std::set<Node*> nodes) {
  for(auto &output : nodes) {
  // This part is for a single output node. Remove if no useful info in this code
//    if(errorAnalyzer->errorComputedNodes[findVarNode(output)->depth].find(symbolTables.find(currentScope)->second->table[output]) ==
//        errorAnalyzer->errorComputedNodes[findVarNode(output)->depth].end()) {
//      errorAnalyzer->errorComputing(findVarNode(output));
//    }
    if(errorAnalyzer->errorComputedNodes[output->depth].find(output) ==
       errorAnalyzer->errorComputedNodes[output->depth].end()) {
      errorAnalyzer->errorComputing(output);
    }

    errorAnalyzer->ErrAccumulator[output] =
            (ibex::ExprNode*) &(*errorAnalyzer->ErrAccumulator[output] *
                                pow(2, -53));
  }

  // This part is for a single output node. Remove if no useful info in this code
//  errorAnalyzer->ErrAccumulator[symbolTables[currentScope]->table[outputs[0]]] =
//          (ibex::ExprNode*) &(*errorAnalyzer->ErrAccumulator[symbolTables[currentScope]->table[outputs[0]]] *
//          pow(2, -53));
}

// Generates Expressions corresponding to all nodes bottom up
void Graph::generateExprDriver(std::set<Node *> nodes) {
//  for (auto &output : outputs) {
//    generateExpr(findVarNode(output));
//  }

    for (auto &node : nodes) {
      generateExpr(node);
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
      errorAnalyzer->parentsOfNode[((UnaryOp *)node)->Operand].insert(node);
//      std::cout << "UnaryOp: " << ((UnaryOp *)node)->expr << std::endl;
      break;
    case NodeType::BINARY_OP:
      generateExpr(((BinaryOp *)node)->leftOperand);
      generateExpr(((BinaryOp *)node)->rightOperand);
      ((BinaryOp *)node)->expr = (ibex::ExprBinaryOp *)&node->generateSymExpr();
      errorAnalyzer->parentsOfNode[((BinaryOp *)node)->leftOperand].insert(node);
      errorAnalyzer->parentsOfNode[((BinaryOp *)node)->rightOperand].insert(node);
//      std::cout << "BinaryOp: " << *((BinaryOp *)node)->expr << std::endl;
      break;
    case NodeType::TERNARY_OP:
      generateExpr(((TernaryOp *)node)->leftOperand);
      generateExpr(((TernaryOp *)node)->middleOperand);
      generateExpr(((TernaryOp *)node)->rightOperand);
//      ((TernaryOp *)node)->expr = (ibex::ExprTernaryOp *)&node->generateSymExpr();
      errorAnalyzer->parentsOfNode[((TernaryOp *)node)->leftOperand].insert(node);
      errorAnalyzer->parentsOfNode[((TernaryOp *)node)->middleOperand].insert(node);
      errorAnalyzer->parentsOfNode[((TernaryOp *)node)->rightOperand].insert(node);
      // Ibex does not have a TernaryOp
      break;
    default:
      std::cout << "Unknown node type" << std::endl;
      break;
  }
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

  // Flatten nodes from children of node within depth window
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
      break;
  }

  return std::set<Node *>();
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
      break;
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
  std::cout << "Nodes with op:" << std::endl;
  for (auto &node : nodes_with_op) {
    std::cout << "\t" << *node << std::endl;
  }

  std::map<Node *, std::set<Node *>> common_dependencies = FindCommonDependencies(nodes_with_op, lower_bound, upper_bound);

  // Print common dependencies
  std::cout << "Common dependencies:" << std::endl;
  for (auto &common_dependency : common_dependencies) {
    std::cout << "\t" << *common_dependency.first << " : ";
    for (auto &node : common_dependency.second) {
      std::cout << *node << " ";
    }
    std::cout << std::endl;
  }

  // Unionize the node set from common_dependencies
  std::set<Node *> common_dependencies_set;
  for (auto &common_dependency : common_dependencies) {
    std::set_union(common_dependencies_set.begin(), common_dependencies_set.end(),
                   common_dependency.second.begin(), common_dependency.second.end(),
                   std::inserter(common_dependencies_set, common_dependencies_set.end()));
  }

  if (common_dependencies_set.empty()) {
    std::cout << "Empty dependence set! Generating candidates!" << std::endl;

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
  std::cout << "Common dependencies set:" << std::endl;
  for (auto &node : common_dependencies_set) {
    std::cout << "\t" << *node << std::endl;
  }

  return common_dependencies_set;
}

std::pair<unsigned int, std::set<Node*>> Graph::selectNodesForAbstraction(unsigned int max_depth,
                                                    unsigned int bound_min_depth,
                                                    unsigned int bound_max_depth) {
  assert(bound_min_depth <= bound_max_depth && bound_max_depth <= max_depth && "Invalid bounds for abstraction");
  std::set<Node*> nodes_to_abstract;

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
    std::cout << "No candidates found!" << std::endl;
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
    std::cout << "Cost Sum Dict:" << std::endl;
    for (auto &cost_sum : cost_sum_dict) {
      std::cout << "\t" << cost_sum.first << " : " << cost_sum.second << std::endl;
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
    std::cout << "Max Depth: " << max_depth << std::endl;
    std::cout << "Abstraction Depth: " << abstraction_depth << std::endl;

    return std::make_pair(abstraction_depth, candidate_nodes);
  }
}



std::map<Node *, std::vector<ibex::IntervalVector>> Graph::performAbstraction(unsigned int bound_min_depth, unsigned int bound_max_depth) {
  // Get max depth using keys in depthTable
  unsigned int max_depth = depthTable.rbegin()->first;

  unsigned int abstraction_level = 1;
  std::cout << "Performing abstraction" << std::endl;

  while (max_depth >= bound_max_depth && max_depth >= bound_min_depth) {
    auto [abstraction_depth, candidate_nodes] = selectNodesForAbstraction(max_depth, bound_min_depth, bound_max_depth);

    if (!candidate_nodes.empty()) {
      // Print candidate nodes
      std::cout << "Candidate Nodes:" << std::endl;
      for (auto &node : candidate_nodes) {
        std::cout << "\t" << *node << std::endl;
      }

      // Modify the AST
      SimplifyWithAbstraction(candidate_nodes, max_depth);


    } else {
      std::cout << "No candidates found!" << std::endl;
      return {};
    }



  }

  // Create a new node for each node in candidateNodes

}

std::map<Node *, std::vector<ibex::IntervalVector>> Graph::SimplifyWithAbstraction(std::set<Node *> nodes, unsigned int max_depth, bool isFinal) {

  for (auto &input: inputs) {
    assert(symbolTables[currentScope]->table[input.first]->isVariable() && "Input is not a variable node");
    ((VariableNode *)symbolTables[currentScope]->table[input.first])->variable = &(ibex::ExprSymbol::new_(input.first.c_str()));
  }

  generateExprDriver(nodes);
  setupDerivativeComputation(nodes);

  errorAnalyzer->derivativeComputingDriver();
  errorComputingDriver(nodes);

  std::map<Node *, std::vector<ibex::IntervalVector>> results;
  for (auto &node : nodes) {
    ibexInterface->setInputIntervals(inputs);
    ibexInterface->setVariables(inputs, symbolTables[currentScope]->table);
    ibexInterface->setFunction(errorAnalyzer->ErrAccumulator[node]);
    results[node].push_back(ibexInterface->eval());
  }

  for (auto &input: inputs) {
    assert(symbolTables[currentScope]->table[input.first]->isVariable() && "Input is not a variable node");
    ((VariableNode *)symbolTables[currentScope]->table[input.first])->variable = &(ibex::ExprSymbol::new_(input.first.c_str()));
  }

  generateExprDriver(nodes);

  for (auto &node : nodes) {
    ibexInterface->setVariables(inputs, symbolTables[currentScope]->table);
    ibexInterface->setFunction(node->getExprNode());
    results[node].push_back(ibexInterface->eval());
  }

  if(isFinal) {
    return results;
  }

  AbstractNodes(results);
  RebuildAST();

  return results;
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
void Graph::AbstractNodes(std::map<Node *, std::vector<ibex::IntervalVector>> results) {
  std::cout << "Abstracting nodes" << std::endl;

  // Turn node in results into VariableNodes and create corresponding FreeVariable nodes
  for (auto &result : results) {
    auto node = result.first;

    VariableNode *converted_node;

    // Convert node to VariableNode
    converted_node = new VariableNode(*node);
    converted_node->setAbsoluteError(&ibex::ExprConstant::new_scalar(result.second[0][0].ub()));

    // Add converted node to nodes and symbol table
    nodes.insert(converted_node);
    symbolTables[currentScope]->table[converted_node->variable->name] = converted_node;

    // Create corresponding FreeVariable node using the result IntervalVector
    auto *free_node = new FreeVariable(result.second[1][0]);
    inputs[converted_node->variable->name] = free_node;

    // Add free node to nodes and inputs
    nodes.insert(free_node);
    free_node->setAbsoluteError(&ibex::ExprConstant::new_scalar(result.second[0][0].ub()));
    free_node->setRounding(converted_node->getRounding());
  }
}

/*
 * Rebuilds the AST post abstraction
 */
void Graph::RebuildAST() {
  std::cout << "Rebuilding AST" << std::endl;

  std::vector<Node*> probe_list = ModProbeList();

  std::map<Node*, unsigned int> completed;

  // Recursively call RebuildASTNode on nodes in probe_list if not already completed
  for (auto &node : probe_list) {
    if (completed.find(node) == completed.end()) {
      RebuildASTNode(node, completed);
    }
  }

  // Get max depth among nodes in probe_list
  unsigned int max_depth = -1;
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
  std::cout << "Num nodes before: " << num_nodes << std::endl;

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
  std::cout << "Num nodes after: " << num_nodes << std::endl;
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
