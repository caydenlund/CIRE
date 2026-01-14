#include "cire/core/Graph.hpp"
#include "interfaces/Logging.h"
#include "parser.h"

#include <algorithm>
#include <utility>

int SymbolTable::SCOPE_COUNTER = 0;

Graph::~Graph() {
    for (const auto& symbolTable : symbolTables) delete symbolTable.second;
    for (const auto& node : nodes) delete node;
    delete errorAnalyzer;
    delete ibexInterface;
}

void Graph::setValidationFile(std::string _validationFile) { validationFile = std::move(_validationFile); }

void Graph::registerLLVMNode(llvm::Value* llvmValue, ir::Node* node) {
    if ((llvmValue != nullptr) && (node != nullptr)) {
        llvmValueToNode[llvmValue] = node;
        nodeToLLVMValue[node] = llvmValue;
    }
}

ir::Node* Graph::getNodeByLLVMValue(llvm::Value* llvmValue) const {
    auto it = llvmValueToNode.find(llvmValue);
    return (it != llvmValueToNode.end()) ? it->second : nullptr;
}

llvm::Value* Graph::getLLVMValueByNode(ir::Node* node) const {
    auto it = nodeToLLVMValue.find(node);
    return (it != nodeToLLVMValue.end()) ? it->second : nullptr;
}

std::vector<ir::Node*> Graph::getNodesByInstructionType(const std::string& type) const {
    auto it = instructionTypeIndex.find(type);
    return (it != instructionTypeIndex.end()) ? it->second : std::vector<ir::Node*>();
}

void Graph::indexNodeByInstructionType(ir::Node* node, const std::string& type) {
    if (node != nullptr) { instructionTypeIndex[type].push_back(node); }
}

std::ostream& operator<<(std::ostream& os, const Graph& graph) {
    graph.write(os);
    return os;
}

void Graph::write(std::ostream& out) const {
    out << "Graph:\n";
    out << "Inputs:\n";
    for (const auto& input : inputs) out << "    " << input.first << " : " << *input.second;
    out << "Outputs:\n";
    for (const auto& output : outputs) out << "    " << output;
    out << "Variables:\n";
    for (const auto& variable : symbolTables.find(currentScope)->second->table) {
        out << "\t" << variable.first << " : " << *variable.second;
    }

    out << "Nodes:\n";
    for (const auto& node : nodes) out << "    " << *node << "\n";

    out << "Depth Table:";
    for (const auto& depth : depthTable) {
        out << "    " << depth.first << " : ";
        for (const auto& node : depth.second) out << *node << " ";
        out << "\n";
    }
}

void Graph::createNewSymbolTable() {
    currentScope = SymbolTable::SCOPE_COUNTER;
    symbolTables[currentScope] = new SymbolTable();
}

void Graph::generateIbexSymbols() {
    for (const auto& input : inputs) {
        assert(symbolTables[currentScope]->table[input.first]->isVariable() && "Input is not a variable node");
        (dynamic_cast<ir::VariableNode*>(symbolTables[currentScope]->table[input.first]))->variable = &(
                ibex::ExprSymbol::new_(input.first.c_str()));
        symbolTables[currentScope]->table[input.first]->setAbsoluteError(
                &ibex::ExprConstant::new_scalar(input.second->var->ub()));
    }


    for (const auto& node : nodes) {
        switch (node->type) {
            case ir::Node::Type::INTEGER: {
                (dynamic_cast<ir::Integer*>(node))->value = &(
                        ibex::ExprConstant::new_scalar((dynamic_cast<ir::Integer*>(node))->val));
                node->setAbsoluteError(&ibex::ExprConstant::new_scalar(0.0));
                break;
            }
            case ir::Node::Type::FLOAT: {
                (dynamic_cast<ir::Float*>(node))->value = &(
                        ibex::ExprConstant::new_scalar((dynamic_cast<ir::Float*>(node))->val));
                node->setAbsoluteError(
                        &ibex::ExprConstant::new_scalar((dynamic_cast<ir::Float*>(node))->val));
                break;
            }
            case ir::Node::Type::DOUBLE: {
                (dynamic_cast<ir::Double*>(node))->value = &(
                        ibex::ExprConstant::new_scalar((dynamic_cast<ir::Double*>(node))->val));
                node->setAbsoluteError(
                        &ibex::ExprConstant::new_scalar((dynamic_cast<ir::Double*>(node))->val));
                break;
            }
            case ir::Node::Type::FREE_VARIABLE: {
                node->setAbsoluteError(&ibex::ExprConstant::new_scalar(dynamic_cast<ir::FreeVariable*>(node)->var->ub()));
                break;
            }
            case ir::Node::Type::VARIABLE:  // The absoluteError has already been set in the previous
                                            // inputs for loop
            // Following nodes do not have an absolute error. Only Constants and FreeVariables have
            // an absolute error
            case ir::Node::Type::UNARY_OP:
            case ir::Node::Type::BINARY_OP:
            case ir::Node::Type::TERNARY_OP:
            case ir::Node::Type::DEFAULT: {
                break;
            }
        }
    }
}

ir::Node* Graph::findFreeVarNode(string Var) const {
    auto it = inputs.find(Var);
    if (it != inputs.end()) return it->second;

    return nullptr;
}

ir::Node* Graph::findVarNode(string Var) const {
    auto it = symbolTables.find(currentScope)->second->table.find(Var);
    if (it != symbolTables.find(currentScope)->second->table.end()) return it->second;

    return nullptr;
}

void Graph::setupDerivativeComputation(std::set<ir::Node*> candidate_nodes) {
    // Set up output
    // Get the max depth of the candidate_nodes
    unsigned int max_depth = 0;
    for (const auto& node : candidate_nodes) max_depth = std::max<unsigned int>(node->depth, max_depth);

    errorAnalyzer->derivativeComputedNodes.clear();
    errorAnalyzer->errorComputedNodes.clear();
    errorAnalyzer->numParentsOfNode.clear();
    errorAnalyzer->parentsOfNode.clear();
    errorAnalyzer->bwdDerivatives.clear();
    errorAnalyzer->typeCastRnd.clear();
    errorAnalyzer->errAccumulator.clear();

    // Insert candidate_nodes with max depth into worklist
    for (const auto& node : candidate_nodes) {
        if (node->depth == max_depth) errorAnalyzer->workList.insert(node);
    }

    // Set BwdDerivatives of each candidate_node (output node) with respect to itself to 1
    for (const auto& node : candidate_nodes) {
        errorAnalyzer->bwdDerivatives[node][node] = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(1);
        errorAnalyzer->typeCastRnd[node][node] = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(0);
    }

    // Set numParentsOfNode of each node to the number of parents it has
    for (const auto& node : candidate_nodes) errorAnalyzer->numParentsOfNode[node] = node->parents.size();
}

// Generates Expressions corresponding to all candidate_nodes bottom up
void Graph::generateExprDriver(const std::set<ir::Node*>& candidate_nodes) {
    // Map from depth to nodes at that depth whose expression has been generated. Similar to
    // "reachable" in Satire
    std::map<int, std::set<ir::Node*>> generatedExprsAtDepth;

    // Map from Ibex::ExprNode to the Nodes that have that expression
    // Common Subexpression Elimination Table
    // This keeps track of nodes that have the same expression and can be replaced by a single node
    std::map<ibex::ExprNode*, std::set<ir::Node*>> cseTable;

    logging->info("Generating expressions...");

    for (const auto& node : candidate_nodes) {
        logging->debug("Processing node '", node->id, "'");
        if (generatedExprsAtDepth[int(node->depth)].find(node) == generatedExprsAtDepth[int(node->depth)].end()) {
            generateExpr(node, generatedExprsAtDepth, cseTable);
        }
        logging->debug("Processed node '", node->id, "'");
    }

    logging->info("Done generating expressions");
}

void Graph::generateExpr(ir::Node* node, std::map<int, std::set<ir::Node*>>& generatedExprsAtDepth,
                         std::map<ibex::ExprNode*, std::set<ir::Node*>>& cseTable) {
    switch (node->type) {
        case ir::Node::Type::INTEGER:
        case ir::Node::Type::FLOAT:
        case ir::Node::Type::DOUBLE:
        case ir::Node::Type::FREE_VARIABLE:
        case ir::Node::Type::VARIABLE:
            // Already has an expression or interval
            break;
        case ir::Node::Type::UNARY_OP:
            if (generatedExprsAtDepth[int(dynamic_cast<ir::UnaryOp*>(node)->operand->depth)].find(
                        dynamic_cast<ir::UnaryOp*>(node)->operand)
                == generatedExprsAtDepth[int(dynamic_cast<ir::UnaryOp*>(node)->operand->depth)].end()) {
                generateExpr(dynamic_cast<ir::UnaryOp*>(node)->operand, generatedExprsAtDepth, cseTable);
            }
            dynamic_cast<ir::UnaryOp*>(node)->expr = dynamic_cast<ibex::ExprUnaryOp*>(&node->generateSymExpr());
            errorAnalyzer->parentsOfNode[dynamic_cast<ir::UnaryOp*>(node)->operand].insert(node);
            logging->debug("Node '", node->id, "' processed");
            logging->debug("    UnaryOp processed");
            break;
        case ir::Node::Type::BINARY_OP:
            if (generatedExprsAtDepth[int(dynamic_cast<ir::BinaryOp*>(node)->leftOperand->depth)].find(
                        dynamic_cast<ir::BinaryOp*>(node)->leftOperand)
                == generatedExprsAtDepth[int(dynamic_cast<ir::BinaryOp*>(node)->leftOperand->depth)].end()) {
                generateExpr(dynamic_cast<ir::BinaryOp*>(node)->leftOperand, generatedExprsAtDepth, cseTable);
            }
            if (generatedExprsAtDepth[int(dynamic_cast<ir::BinaryOp*>(node)->rightOperand->depth)].find(
                        dynamic_cast<ir::BinaryOp*>(node)->rightOperand)
                == generatedExprsAtDepth[int(dynamic_cast<ir::BinaryOp*>(node)->rightOperand->depth)].end()) {
                generateExpr(dynamic_cast<ir::BinaryOp*>(node)->rightOperand, generatedExprsAtDepth, cseTable);
            }
            dynamic_cast<ir::BinaryOp*>(node)->expr = dynamic_cast<ibex::ExprBinaryOp*>(&node->generateSymExpr());
            errorAnalyzer->parentsOfNode[dynamic_cast<ir::BinaryOp*>(node)->leftOperand].insert(node);
            errorAnalyzer->parentsOfNode[dynamic_cast<ir::BinaryOp*>(node)->rightOperand].insert(node);
            logging->debug("Node '", node->id, "' processed");
            logging->debug("    BinaryOp processed");
            break;
        case ir::Node::Type::TERNARY_OP:
            if (generatedExprsAtDepth[int(dynamic_cast<ir::TernaryOp*>(node)->leftOperand->depth)].find(
                        dynamic_cast<ir::TernaryOp*>(node)->leftOperand)
                == generatedExprsAtDepth[int(dynamic_cast<ir::TernaryOp*>(node)->leftOperand->depth)].end()) {
                generateExpr(dynamic_cast<ir::TernaryOp*>(node)->leftOperand, generatedExprsAtDepth, cseTable);
            }
            if (generatedExprsAtDepth[int(dynamic_cast<ir::TernaryOp*>(node)->middleOperand->depth)].find(
                        dynamic_cast<ir::TernaryOp*>(node)->middleOperand)
                == generatedExprsAtDepth[int(dynamic_cast<ir::TernaryOp*>(node)->middleOperand->depth)].end()) {
                generateExpr(dynamic_cast<ir::TernaryOp*>(node)->middleOperand, generatedExprsAtDepth, cseTable);
            }
            if (generatedExprsAtDepth[int(dynamic_cast<ir::TernaryOp*>(node)->rightOperand->depth)].find(
                        dynamic_cast<ir::TernaryOp*>(node)->rightOperand)
                == generatedExprsAtDepth[int(dynamic_cast<ir::TernaryOp*>(node)->rightOperand->depth)].end()) {
                generateExpr(dynamic_cast<ir::TernaryOp*>(node)->rightOperand, generatedExprsAtDepth, cseTable);
            }
            // Ibex does not have a TernaryOp, so we split the Op into two BinaryOps
            dynamic_cast<ir::TernaryOp*>(node)->expr = dynamic_cast<ibex::ExprBinaryOp*>(&node->generateSymExpr());
            errorAnalyzer->parentsOfNode[dynamic_cast<ir::TernaryOp*>(node)->leftOperand].insert(node);
            errorAnalyzer->parentsOfNode[dynamic_cast<ir::TernaryOp*>(node)->middleOperand].insert(node);
            errorAnalyzer->parentsOfNode[dynamic_cast<ir::TernaryOp*>(node)->rightOperand].insert(node);
            logging->debug("Node '", node->id, "' processed");
            logging->debug("    TernaryOp processed");
            break;
        default:
            logging->critical("Unknown node type");
    }

    // Update the map tracking processed nodes
    generatedExprsAtDepth[int(node->depth)].insert(node);

    // Common sub expression elimination phase
    // 1) Find all subexpressions similar to the current node
    std::vector<ir::Node*> nodes_to_merge;
    for (const auto& n : cseTable[node->getExprNode()]) {
        // Ensuring n and node are not the same nodes
        if (n != node) {
            // Check if all children of n and node are the same
            switch (n->type) {
                case ir::Node::Type::INTEGER:
                    if (node->isInteger()) nodes_to_merge.push_back(n);
                    break;
                case ir::Node::Type::FLOAT:
                    if (node->isFloat()) nodes_to_merge.push_back(n);
                    break;
                case ir::Node::Type::DOUBLE:
                    if (node->isDouble()) nodes_to_merge.push_back(n);
                    break;
                case ir::Node::Type::FREE_VARIABLE:
                    if (node->isFreeVariable()) nodes_to_merge.push_back(n);
                    break;
                case ir::Node::Type::VARIABLE:
                    if (node->isVariable()) nodes_to_merge.push_back(n);
                    break;
                case ir::Node::Type::UNARY_OP:
                    if (node->isUnaryOp()
                        && dynamic_cast<ir::UnaryOp*>(n)->operand == dynamic_cast<ir::UnaryOp*>(node)->operand) {
                        nodes_to_merge.push_back(n);
                    }
                    break;
                case ir::Node::Type::BINARY_OP:
                    if (node->isBinaryOp()
                        && dynamic_cast<ir::BinaryOp*>(n)->leftOperand == dynamic_cast<ir::BinaryOp*>(node)->leftOperand
                        && dynamic_cast<ir::BinaryOp*>(n)->rightOperand
                                   == dynamic_cast<ir::BinaryOp*>(node)->rightOperand) {
                        nodes_to_merge.push_back(n);
                    }
                    break;
                case ir::Node::Type::TERNARY_OP:
                    if (node->isTernaryOp()
                        && dynamic_cast<ir::TernaryOp*>(n)->leftOperand
                                   == dynamic_cast<ir::TernaryOp*>(node)->leftOperand
                        && dynamic_cast<ir::TernaryOp*>(n)->middleOperand
                                   == dynamic_cast<ir::TernaryOp*>(node)->middleOperand
                        && dynamic_cast<ir::TernaryOp*>(n)->rightOperand
                                   == dynamic_cast<ir::TernaryOp*>(node)->rightOperand) {
                        nodes_to_merge.push_back(n);
                    }
                    break;
                default:
                    logging->critical("Unknown node type");
            }
        }
    }

    // 2) Merge all similar subexpressions
    if (nodes_to_merge.empty()) {
        cseTable[node->getExprNode()].insert(node);
    } else {
        logging->debug("Found ", nodes_to_merge.size(), " nodes to merge with node '", node->id, "'");
        for (const auto& n : nodes_to_merge) {
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
ir::Node* Graph::mergeNodes(ir::Node* node1, ir::Node* node2, std::map<ir::Node*, std::set<ir::Node*>>& parentsOfNode) {
    // Set union of parents of node1 and node2
    std::set<ir::Node*> new_parents;
    std::set_union(parentsOfNode[node1].begin(), parentsOfNode[node1].end(), parentsOfNode[node2].begin(),
                   parentsOfNode[node2].end(), std::inserter(new_parents, new_parents.end()));

    // Update the children of the new_parents set to point to node2 of they point to node1
    for (const auto& par : new_parents) {
        switch (par->type) {
            case ir::Node::Type::INTEGER:
            case ir::Node::Type::FLOAT:
            case ir::Node::Type::DOUBLE:
            case ir::Node::Type::FREE_VARIABLE:
            case ir::Node::Type::VARIABLE:
                break;
            case ir::Node::Type::UNARY_OP:
                if (dynamic_cast<ir::UnaryOp*>(par)->operand == node1) dynamic_cast<ir::UnaryOp*>(par)->operand = node2;
                break;
            case ir::Node::Type::BINARY_OP:
                if (dynamic_cast<ir::BinaryOp*>(par)->leftOperand == node1) {
                    dynamic_cast<ir::BinaryOp*>(par)->leftOperand = node2;
                }
                if (dynamic_cast<ir::BinaryOp*>(par)->rightOperand == node1) {
                    dynamic_cast<ir::BinaryOp*>(par)->rightOperand = node2;
                }
                break;
            case ir::Node::Type::TERNARY_OP:
                if ((dynamic_cast<ir::TernaryOp*>(par))->leftOperand == node1) {
                    (dynamic_cast<ir::TernaryOp*>(par))->leftOperand = node2;
                }
                if ((dynamic_cast<ir::TernaryOp*>(par))->middleOperand == node1) {
                    (dynamic_cast<ir::TernaryOp*>(par))->middleOperand = node2;
                }
                if ((dynamic_cast<ir::TernaryOp*>(par))->rightOperand == node1) {
                    (dynamic_cast<ir::TernaryOp*>(par))->rightOperand = node2;
                }
                break;
            default:
                logging->critical("Unknown node type");
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
        case ir::Node::Type::INTEGER:
        case ir::Node::Type::FLOAT:
        case ir::Node::Type::DOUBLE:
        case ir::Node::Type::FREE_VARIABLE:
        case ir::Node::Type::VARIABLE:
        case ir::Node::Type::DEFAULT:
            break;
        case ir::Node::Type::UNARY_OP:
            parentsOfNode[(dynamic_cast<ir::UnaryOp*>(node2))->operand].erase(node1);
            break;
        case ir::Node::Type::BINARY_OP:
            parentsOfNode[(dynamic_cast<ir::BinaryOp*>(node2))->leftOperand].erase(node1);
            parentsOfNode[(dynamic_cast<ir::BinaryOp*>(node2))->rightOperand].erase(node1);
            break;
        case ir::Node::Type::TERNARY_OP:
            parentsOfNode[(dynamic_cast<ir::TernaryOp*>(node2))->leftOperand].erase(node1);
            parentsOfNode[(dynamic_cast<ir::TernaryOp*>(node2))->middleOperand].erase(node1);
            parentsOfNode[(dynamic_cast<ir::TernaryOp*>(node2))->rightOperand].erase(node1);
            break;
    }

    // Remove `node1` from parentsOfNode
    parentsOfNode.erase(node1);

    return node2;
}

void Graph::concretizeErrorComponents() {
    int totalNodesInBwdDerivatives = int(errorAnalyzer->bwdDerivatives.size());
    int processedNodes = 0;

    // Iterate through the bwdDerivative map
    for (const auto& node_bwd_derivatives : errorAnalyzer->bwdDerivatives) {
        ir::Node* node = node_bwd_derivatives.first;

        node_bwd_derivatives.second.size();

        logging->debug("Processing Node '", node->id, "'. Processed ", processedNodes, "/", totalNodesInBwdDerivatives);

        // Iterate through the nodeBwdDerivatives map
        for (const auto& node_output_bwd_derivative : node_bwd_derivatives.second) {
            ir::Node* output_node = node_output_bwd_derivative.first;
            OptResult max_bwd = ibexInterface->findAbsMax(*node_output_bwd_derivative.second);
            OptResult max_local_err = ibexInterface->findAbsMax(
                    const_cast<ibex::ExprNode&>(product(node->getAbsoluteError(), node->getRounding())));

            errorAnalyzer->bwdDerivatives[node][output_node] = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(
                    (-max_bwd.result).mag());
            node->setAbsoluteError((ibex::ExprNode*)&ibex::ExprConstant::new_scalar((-max_local_err.result).mag()));
        }

        processedNodes++;
    }
}

// Uses ibex eval to evaluate the backward derivatives and local errors and stores them in a map of
// node to ibex::IntervalVector, ibex::IntervalVector
void Graph::examineBwdDerivativeAndLocalError() {
    // Create a map similar to errorAnalyzer->BwdDerivatives but with a pair of double as the value
    std::map<ir::Node*, std::map<ir::Node*, std::pair<double, double>>> evaluatedBwdDerivatives;

    // Iterate through the bwdDerivative map
    for (const auto& node_bwd_derivatives : errorAnalyzer->bwdDerivatives) {
        ir::Node* node = node_bwd_derivatives.first;
        // Iterate through the nodeBwdDerivatives map
        for (const auto& node_output_bwd_derivative : node_bwd_derivatives.second) {
            ir::Node* output_node = node_output_bwd_derivative.first;
            OptResult max_bwd = ibexInterface->findAbsMax(*node_output_bwd_derivative.second);
            OptResult max_local_err = ibexInterface->findAbsMax(
                    const_cast<ibex::ExprNode&>(product(node->getAbsoluteError(), node->getRounding())));

            evaluatedBwdDerivatives[node][output_node] = std::make_pair((-max_bwd.result).mag(),
                                                                        (-max_local_err.result).mag());
        }
    }

    // print the evaluatedBwdDerivatives
    for (const auto& node_bwd_derivatives : evaluatedBwdDerivatives) {
        ir::Node* node = node_bwd_derivatives.first;
        for (const auto& node_output_bwd_derivative : node_bwd_derivatives.second) {
            ir::Node* output_node = node_output_bwd_derivative.first;
            std::pair<double, double> bwd_local_err = node_output_bwd_derivative.second;
            logging->debug("(Output Id, Depth): ", output_node->id, ", ", output_node->depth,
                           " (Node Id, Depth): ", node->id, ", ", node->depth, " Bwd: ", bwd_local_err.first,
                           " Local Error: ", bwd_local_err.second);
        }
    }

    // Store the evaluatedBwdDerivatives in a file
    std::ofstream bwd_derivatives_file("bwd_derivatives.csv");
    // Output format: Node Id, Depth, Bwd, Local Error
    bwd_derivatives_file << "Node Id,Depth,Bwd,Local Error";
    for (const auto& node_bwd_derivatives : evaluatedBwdDerivatives) {
        ir::Node* node = node_bwd_derivatives.first;
        for (const auto& node_output_bwd_derivative : node_bwd_derivatives.second) {
            ir::Node* output_node = node_output_bwd_derivative.first;
            std::pair<double, double> bwd_local_err = node_output_bwd_derivative.second;
            bwd_derivatives_file << node->id << "," << node->depth << "," << bwd_local_err.first << ","
                                 << bwd_local_err.second;
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
                return compareDAGs((dynamic_cast<ibex::ExprAdd*>(&expr1))->left,
                                   (dynamic_cast<ibex::ExprAdd*>(&expr2))->left)
                    && compareDAGs((dynamic_cast<ibex::ExprAdd*>(&expr1))->right,
                                   (dynamic_cast<ibex::ExprAdd*>(&expr2))->right);
            case ibex::ExprNode::NumExprMul:
                return compareDAGs((dynamic_cast<ibex::ExprMul*>(&expr1))->left,
                                   (dynamic_cast<ibex::ExprMul*>(&expr2))->left)
                    && compareDAGs((dynamic_cast<ibex::ExprMul*>(&expr1))->right,
                                   (dynamic_cast<ibex::ExprMul*>(&expr2))->right);
            case ibex::ExprNode::NumExprSub:
                return compareDAGs((dynamic_cast<ibex::ExprSub*>(&expr1))->left,
                                   (dynamic_cast<ibex::ExprSub*>(&expr2))->left)
                    && compareDAGs((dynamic_cast<ibex::ExprSub*>(&expr1))->right,
                                   (dynamic_cast<ibex::ExprSub*>(&expr2))->right);
            case ibex::ExprNode::NumExprDiv:
                return compareDAGs((dynamic_cast<ibex::ExprDiv*>(&expr1))->left,
                                   (dynamic_cast<ibex::ExprDiv*>(&expr2))->left)
                    && compareDAGs((dynamic_cast<ibex::ExprDiv*>(&expr1))->right,
                                   (dynamic_cast<ibex::ExprDiv*>(&expr2))->right);
            case ibex::ExprNode::NumExprSin:
                return compareDAGs((dynamic_cast<ibex::ExprSin*>(&expr1))->expr,
                                   (dynamic_cast<ibex::ExprSin*>(&expr2))->expr);
            case ibex::ExprNode::NumExprCos:
                return compareDAGs((dynamic_cast<ibex::ExprCos*>(&expr1))->expr,
                                   (dynamic_cast<ibex::ExprCos*>(&expr2))->expr);
            case ibex::ExprNode::NumExprTan:
                return compareDAGs((dynamic_cast<ibex::ExprTan*>(&expr1))->expr,
                                   (dynamic_cast<ibex::ExprTan*>(&expr2))->expr);
            case ibex::ExprNode::NumExprSinh:
                return compareDAGs((dynamic_cast<ibex::ExprSinh*>(&expr1))->expr,
                                   (dynamic_cast<ibex::ExprSinh*>(&expr2))->expr);
            case ibex::ExprNode::NumExprCosh:
                return compareDAGs((dynamic_cast<ibex::ExprCosh*>(&expr1))->expr,
                                   (dynamic_cast<ibex::ExprCosh*>(&expr2))->expr);
            case ibex::ExprNode::NumExprTanh:
                return compareDAGs((dynamic_cast<ibex::ExprTanh*>(&expr1))->expr,
                                   (dynamic_cast<ibex::ExprTanh*>(&expr2))->expr);
            case ibex::ExprNode::NumExprAsin:
                return compareDAGs((dynamic_cast<ibex::ExprAsin*>(&expr1))->expr,
                                   (dynamic_cast<ibex::ExprAsin*>(&expr2))->expr);
            case ibex::ExprNode::NumExprAcos:
                return compareDAGs((dynamic_cast<ibex::ExprAcos*>(&expr1))->expr,
                                   (dynamic_cast<ibex::ExprAcos*>(&expr2))->expr);
            case ibex::ExprNode::NumExprAtan:
                return compareDAGs((dynamic_cast<ibex::ExprAtan*>(&expr1))->expr,
                                   (dynamic_cast<ibex::ExprAtan*>(&expr2))->expr);
            case ibex::ExprNode::NumExprLog:
                return compareDAGs((dynamic_cast<ibex::ExprLog*>(&expr1))->expr,
                                   (dynamic_cast<ibex::ExprLog*>(&expr2))->expr);
            case ibex::ExprNode::NumExprSqrt:
                return compareDAGs((dynamic_cast<ibex::ExprSqrt*>(&expr1))->expr,
                                   (dynamic_cast<ibex::ExprSqrt*>(&expr2))->expr);
            case ibex::ExprNode::NumExprExp:
                return compareDAGs((dynamic_cast<ibex::ExprExp*>(&expr1))->expr,
                                   (dynamic_cast<ibex::ExprExp*>(&expr2))->expr);
            default:
                logging->critical("Unknown node type");
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
std::set<ir::Node*> Graph::flattenSubDags(ir::Node* node, unsigned int min_depth, unsigned int max_depth) {
    assert(min_depth <= max_depth && "Invalid bounds for flattening");

    std::set<ir::Node*> nodes_to_flatten;

    // Flatten nodes from children of node within depth windowI
    switch (node->type) {
        case ir::Node::Type::INTEGER:
        case ir::Node::Type::FLOAT:
        case ir::Node::Type::DOUBLE:
        case ir::Node::Type::FREE_VARIABLE:
        case ir::Node::Type::VARIABLE:
            break;
        case ir::Node::Type::UNARY_OP:
            if ((dynamic_cast<ir::UnaryOp*>(node))->operand->depth >= min_depth
                && (dynamic_cast<ir::UnaryOp*>(node))->operand->depth <= max_depth) {
                nodes_to_flatten.insert((dynamic_cast<ir::UnaryOp*>(node))->operand);
            }
            if ((dynamic_cast<ir::UnaryOp*>(node))->operand->depth > min_depth) {
                std::set_union(
                        nodes_to_flatten.begin(), nodes_to_flatten.end(),
                        flattenSubDags((dynamic_cast<ir::UnaryOp*>(node))->operand, min_depth, max_depth).begin(),
                        flattenSubDags((dynamic_cast<ir::UnaryOp*>(node))->operand, min_depth, max_depth).end(),
                        std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
            }
            break;
        case ir::Node::Type::BINARY_OP:
            if ((dynamic_cast<ir::BinaryOp*>(node))->leftOperand->depth >= min_depth
                && (dynamic_cast<ir::BinaryOp*>(node))->leftOperand->depth <= max_depth) {
                nodes_to_flatten.insert((dynamic_cast<ir::BinaryOp*>(node))->leftOperand);
            }
            if ((dynamic_cast<ir::BinaryOp*>(node))->rightOperand->depth >= min_depth
                && (dynamic_cast<ir::BinaryOp*>(node))->rightOperand->depth <= max_depth) {
                nodes_to_flatten.insert((dynamic_cast<ir::BinaryOp*>(node))->rightOperand);
            }
            if ((dynamic_cast<ir::BinaryOp*>(node))->leftOperand->depth > min_depth) {
                std::set_union(
                        nodes_to_flatten.begin(), nodes_to_flatten.end(),
                        flattenSubDags((dynamic_cast<ir::BinaryOp*>(node))->leftOperand, min_depth, max_depth).begin(),
                        flattenSubDags((dynamic_cast<ir::BinaryOp*>(node))->leftOperand, min_depth, max_depth).end(),
                        std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
            }
            if ((dynamic_cast<ir::BinaryOp*>(node))->rightOperand->depth > min_depth) {
                std::set_union(
                        nodes_to_flatten.begin(), nodes_to_flatten.end(),
                        flattenSubDags((dynamic_cast<ir::BinaryOp*>(node))->rightOperand, min_depth, max_depth).begin(),
                        flattenSubDags((dynamic_cast<ir::BinaryOp*>(node))->rightOperand, min_depth, max_depth).end(),
                        std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
            }
            break;
        case ir::Node::Type::TERNARY_OP:
            if ((dynamic_cast<ir::TernaryOp*>(node))->leftOperand->depth >= min_depth
                && (dynamic_cast<ir::TernaryOp*>(node))->leftOperand->depth <= max_depth) {
                nodes_to_flatten.insert((dynamic_cast<ir::TernaryOp*>(node))->leftOperand);
            }
            if ((dynamic_cast<ir::TernaryOp*>(node))->middleOperand->depth >= min_depth
                && (dynamic_cast<ir::TernaryOp*>(node))->middleOperand->depth <= max_depth) {
                nodes_to_flatten.insert((dynamic_cast<ir::TernaryOp*>(node))->middleOperand);
            }
            if ((dynamic_cast<ir::TernaryOp*>(node))->rightOperand->depth >= min_depth
                && (dynamic_cast<ir::TernaryOp*>(node))->rightOperand->depth <= max_depth) {
                nodes_to_flatten.insert((dynamic_cast<ir::TernaryOp*>(node))->rightOperand);
            }
            if ((dynamic_cast<ir::TernaryOp*>(node))->leftOperand->depth > min_depth) {
                std::set_union(
                        nodes_to_flatten.begin(), nodes_to_flatten.end(),
                        flattenSubDags((dynamic_cast<ir::TernaryOp*>(node))->leftOperand, min_depth, max_depth).begin(),
                        flattenSubDags((dynamic_cast<ir::TernaryOp*>(node))->leftOperand, min_depth, max_depth).end(),
                        std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
            }
            if ((dynamic_cast<ir::TernaryOp*>(node))->middleOperand->depth > min_depth) {
                std::set_union(
                        nodes_to_flatten.begin(), nodes_to_flatten.end(),
                        flattenSubDags((dynamic_cast<ir::TernaryOp*>(node))->middleOperand, min_depth, max_depth)
                                .begin(),
                        flattenSubDags((dynamic_cast<ir::TernaryOp*>(node))->middleOperand, min_depth, max_depth).end(),
                        std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
            }
            if ((dynamic_cast<ir::TernaryOp*>(node))->rightOperand->depth > min_depth) {
                std::set_union(
                        nodes_to_flatten.begin(), nodes_to_flatten.end(),
                        flattenSubDags((dynamic_cast<ir::TernaryOp*>(node))->rightOperand, min_depth, max_depth)
                                .begin(),
                        flattenSubDags((dynamic_cast<ir::TernaryOp*>(node))->rightOperand, min_depth, max_depth).end(),
                        std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
            }
            break;
        default:
            logging->critical("Unknown node type");
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
std::set<ir::Node*> Graph::findCommonNodes(ir::Node* node, unsigned int min_depth, unsigned int max_depth) {
    std::set<ir::Node*> common_nodes;

    // Create a list of flattened subDAGs of children of node
    std::vector<std::set<ir::Node*>> flattened_subDAGs;
    switch (node->type) {
        case ir::Node::Type::INTEGER:
        case ir::Node::Type::FLOAT:
        case ir::Node::Type::DOUBLE:
        case ir::Node::Type::FREE_VARIABLE:
        case ir::Node::Type::VARIABLE:
            break;
        case ir::Node::Type::UNARY_OP:
            flattened_subDAGs.push_back(
                    flattenSubDags((dynamic_cast<ir::UnaryOp*>(node))->operand, min_depth, max_depth));
            break;
        case ir::Node::Type::BINARY_OP:
            flattened_subDAGs.push_back(
                    flattenSubDags((dynamic_cast<ir::BinaryOp*>(node))->leftOperand, min_depth, max_depth));
            flattened_subDAGs.push_back(
                    flattenSubDags((dynamic_cast<ir::BinaryOp*>(node))->rightOperand, min_depth, max_depth));
            break;
        case ir::Node::Type::TERNARY_OP:
            flattened_subDAGs.push_back(
                    flattenSubDags((dynamic_cast<ir::TernaryOp*>(node))->leftOperand, min_depth, max_depth));
            flattened_subDAGs.push_back(
                    flattenSubDags((dynamic_cast<ir::TernaryOp*>(node))->middleOperand, min_depth, max_depth));
            flattened_subDAGs.push_back(
                    flattenSubDags((dynamic_cast<ir::TernaryOp*>(node))->rightOperand, min_depth, max_depth));
            break;
        default:
            logging->critical("Unknown node type");
    }

    flattened_subDAGs.push_back(std::set<ir::Node*>({node}));

    // Find common nodes
    for (const auto& flattened_subDAG : flattened_subDAGs) {
        if (common_nodes.empty()) {
            common_nodes = flattened_subDAG;
        } else {
            std::set<ir::Node*> temp;
            std::set_intersection(common_nodes.begin(), common_nodes.end(), flattened_subDAG.begin(),
                                  flattened_subDAG.end(), std::inserter(temp, temp.end()));
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
std::map<ir::Node*, std::set<ir::Node*>> Graph::findCommonDependencies(std::set<ir::Node*> nodes,
                                                                       unsigned int min_depth, unsigned int max_depth) {
    std::map<ir::Node*, std::set<ir::Node*>> common_dependencies;

    // Populate common_dependencies with common_nodes from each node's common node list
    for (const auto& node : nodes) {
        std::set<ir::Node*> initial_dependence_list = findCommonNodes(node, min_depth, max_depth);
        std::vector<std::set<ir::Node*>> common_nodes_list;

        // Populate common_nodes_list with common_nodes from each node's common node list
        common_nodes_list.reserve(initial_dependence_list.size());
        for (const auto& node : initial_dependence_list) {
            common_nodes_list.push_back(findCommonNodes(node, min_depth, max_depth));
        }

        std::set<ir::Node*> redundant_nodes;

        // Unionize common_nodes_list into redundant_nodes
        for (const auto& common_nodes : common_nodes_list) {
            std::set_union(redundant_nodes.begin(), redundant_nodes.end(), common_nodes.begin(), common_nodes.end(),
                           std::inserter(redundant_nodes, redundant_nodes.end()));
        }

        // Get the set difference between initial_dependence_list and redundant_nodes
        std::set<ir::Node*> common_nodes;
        std::set_difference(initial_dependence_list.begin(), initial_dependence_list.end(), redundant_nodes.begin(),
                            redundant_nodes.end(), std::inserter(common_nodes, common_nodes.end()));
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
std::set<ir::Node*> Graph::filterNodesWithOperationWithinDepth(ir::Node::OpType op, unsigned int max_depth) {
    std::set<ir::Node*> nodes_with_op;

    std::copy_if(nodes.begin(), nodes.end(), std::inserter(nodes_with_op, nodes_with_op.end()),
                 [op, max_depth](ir::Node* node) {
                     return node->depth <= max_depth
                                 && (node->isUnaryOp() && (dynamic_cast<ir::UnaryOp*>(node))->op == op)
                         || (node->isBinaryOp() && (dynamic_cast<ir::BinaryOp*>(node))->op == op);
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
std::set<ir::Node*> Graph::filterCandidatesForAbstraction(unsigned int max_depth, unsigned int lower_bound,
                                                          unsigned int upper_bound) {
    assert(lower_bound <= upper_bound && upper_bound <= max_depth && "Invalid bounds for abstraction");

    std::set<ir::Node*> nodes_with_op = filterNodesWithOperationWithinDepth(ir::Node::OpType::DIV, max_depth);

    // Print nodes with op
    logging->debug("Nodes with op:");
    for (const auto& node : nodes_with_op) logging->debug("    Node ID: ", node->id);

    std::map<ir::Node*, std::set<ir::Node*>> common_dependencies = findCommonDependencies(nodes_with_op, lower_bound,
                                                                                          upper_bound);

    // Print common dependencies
    logging->debug("Common dependencies:");
    for (const auto& common_dependency : common_dependencies) {
        logging->debug("    Node ID: ", common_dependency.first->id, ":");
        for (const auto& node : common_dependency.second) logging->debug("        Node ID: ", node->id);
    }

    // Unionize the node set from common_dependencies
    std::set<ir::Node*> common_dependencies_set;
    for (const auto& common_dependency : common_dependencies) {
        std::set_union(common_dependencies_set.begin(), common_dependencies_set.end(), common_dependency.second.begin(),
                       common_dependency.second.end(),
                       std::inserter(common_dependencies_set, common_dependencies_set.end()));
    }

    if (common_dependencies_set.empty()) {
        logging->debug("Empty dependence set! Generating candidates!");

        // Get all nodes from depthTable within the depth window with node type UnaryOp, BinaryOp,
        // or TernaryOp
        for (const auto& depth_table : depthTable) {
            if (depth_table.first >= lower_bound && depth_table.first <= upper_bound) {
                std::set_union(common_dependencies_set.begin(), common_dependencies_set.end(),
                               depth_table.second.begin(), depth_table.second.end(),
                               std::inserter(common_dependencies_set, common_dependencies_set.end()));
            }
        }
    } else {
        unsigned int local_max_depth = -1;
        // Get the greatest depth from nodes in common_dependencies_set
        for (const auto& node : common_dependencies_set) local_max_depth = std::max(node->depth, local_max_depth);

        // Get nodes from common_dependencies_set with depth equal to local_max_depth
        std::set<ir::Node*> common_dependencies_set;
        for (const auto& node : common_dependencies_set) {
            if (node->depth == local_max_depth) common_dependencies_set.insert(node);
        }
    }

    // Print common dependencies set
    logging->debug("Common dependencies set:");
    for (const auto& node : common_dependencies_set) logging->debug("    Node ID: ", node->id);

    return common_dependencies_set;
}

std::pair<unsigned int, std::set<ir::Node*>>
Graph::selectNodesForAbstraction(unsigned int max_depth, unsigned int bound_min_depth, unsigned int bound_max_depth) {
    assert(bound_min_depth <= bound_max_depth && bound_max_depth <= max_depth && "Invalid bounds for abstraction");
    std::set<ir::Node*> nodes_to_abstract;

    logging->debug("Selecting nodes for abstraction...");

    // Abstraction window is just 1 level wide
    if (bound_min_depth == bound_max_depth && bound_max_depth <= max_depth) {
        return std::make_pair(bound_min_depth, depthTable[int(bound_min_depth)]);
    }

    std::set<ir::Node*> initialCandidateList = filterCandidatesForAbstraction(max_depth, bound_min_depth,
                                                                              bound_max_depth);

    unsigned int local_max_depth = bound_max_depth;


    // Keep increasing local_max_depth until initialCandidateList is not empty
    while (initialCandidateList.empty() && local_max_depth <= max_depth) {
        local_max_depth += 5;
        initialCandidateList = filterCandidatesForAbstraction(max_depth, bound_min_depth, local_max_depth);
    }

    if (initialCandidateList.empty()) {
        logging->debug("No candidates found!");
        return std::make_pair(-1, std::set<ir::Node*>());
    }
    local_max_depth = 0;
    // Set local_max_depth to the greatest depth of nodes in initialCandidateList
    for (const auto& node : initialCandidateList) local_max_depth = std::max(node->depth, local_max_depth);

    auto f = [&local_max_depth](ir::Node* x) { return float(x->depth) / (local_max_depth + 0.01); };

    auto g = [](ir::Node* x, auto y) { return (-1) * y * log2(y) * x->parents.size(); };

    // Create a list of cost
    std::map<ir::Node*, double> cost_dict;

    // Compute g(x, f(x)) for each node in initialCandidateList
    for (const auto& node : initialCandidateList) cost_dict[node] = g(node, f(node));

    // Sum cost of all nodes with same depth
    std::map<int, double> cost_sum_dict;
    for (const auto& node : initialCandidateList) cost_sum_dict[int(node->depth)] += cost_dict[node];

    // Print cost_sum_dict
    logging->debug("Cost Sum Dict:");
    for (const auto& cost_sum : cost_sum_dict) logging->debug("\t", cost_sum.first, " : ", cost_sum.second);

    // Get the depth with the greatest cost
    int abstraction_depth = -1;
    double greatest_cost = -1;
    for (const auto& cost_sum : cost_sum_dict) {
        if (cost_sum.second > greatest_cost) {
            greatest_cost = cost_sum.second;
            abstraction_depth = cost_sum.first;
        }
    }

    // Get nodes with depth equal to depth_with_greatest_cost
    auto candidate_nodes = depthTable[abstraction_depth];

    // Print max depth and abstraction depth
    logging->debug("Max Depth: ", max_depth);
    logging->debug("Abstraction Depth: ", abstraction_depth);

    return std::make_pair(abstraction_depth, candidate_nodes);
}


void Graph::performAbstraction(unsigned int bound_min_depth, unsigned int bound_max_depth) {
    // Get max depth using keys in depthTable
    unsigned int max_depth = depthTable.rbegin()->first;

    unsigned int abstraction_count = 1;

    logging->debug("Performing abstraction with window [", bound_min_depth, ", ", bound_max_depth, "]");


    while (max_depth >= bound_max_depth && max_depth >= bound_min_depth) {
        auto [abstraction_depth, candidate_nodes] = selectNodesForAbstraction(max_depth, bound_min_depth,
                                                                              bound_max_depth);

        logging->debug("Abstraction count: ", abstraction_count);

        if (!candidate_nodes.empty()) {
            // Print candidate nodes
            logging->debug("Abstraction count: ", abstraction_count);
            logging->debug("Candidate Nodes:");
            for (const auto& node : candidate_nodes) logging->debug("    Node ID: ", node->id);
        }

        if (!candidate_nodes.empty()) {
            abstraction_count++;
            abstractionMetrics[abstraction_count]["bound_min"] = bound_min_depth;
            abstractionMetrics[abstraction_count]["bound_max"] = bound_max_depth;
            abstractionMetrics[abstraction_count]["abstraction_depth"] = abstraction_depth;
            abstractionMetrics[abstraction_count]["num_candidate_nodes"] = candidate_nodes.size();

            // Modify the AST
            simplifyWithAbstraction(candidate_nodes, max_depth);

            max_depth = depthTable.rbegin()->first;

            // The expressions have been built to this point, so we can query the IBEX expression
            // for op counts
            unsigned int max_operators_count = 1000;
            for (const auto& node : candidate_nodes) {
                unsigned int op_count = node->getExprNode()->size;
                max_operators_count = std::min(op_count, max_operators_count);
            }

            // These metrics are computed after the abstraction is performed
            abstractionMetrics[abstraction_count]["max_operators_count"] = max_operators_count;
            abstractionMetrics[abstraction_count]["max_depth"] = max_depth;

            if (max_operators_count < 1000 && max_depth > 8 && bound_min_depth > 5) {
                if (bound_max_depth > max_depth) {
                    bound_max_depth = max_depth;
                } else if (bound_max_depth - bound_min_depth > 4) {
                    bound_max_depth = bound_max_depth - 2;
                }

                if (bound_max_depth - bound_min_depth > 4) bound_min_depth = bound_min_depth - 2;
            } else if (max_depth <= bound_max_depth && max_depth > bound_min_depth) {
                bound_max_depth = max_depth;
                assert(bound_max_depth >= bound_min_depth);
            }
        } else {
            logging->debug("No candidates found!");
        }
    }

    logging->debug("Abstraction complete!");
}

void Graph::findOutputExtrema(const std::set<ir::Node*>& candidate_nodes) {
    logging->debug("Finding output extremas for ", candidate_nodes.size(), " nodes");


    std::map<ir::Node*, OptResult> max;
    for (const auto& node : candidate_nodes) {
        logging->debug("Finding max for '", node->id, "'");
        logging->debug("Output Expression computed for node ", node->id);
        max[node] = ibexInterface->findAbsMax(*node->getExprNode());

        // print the output interval - Max have to be flipped since we find the min of the negative
        // of the function
        logging->debug("Max Interval: ", -max[node].result);
    }

    for (const auto& node : candidate_nodes) {
        logging->debug("Output Extrema computed for node ", node->id);

        errorAnalysisResults[node].outputExtrema = -max[node].result;
        errorAnalysisResults[node].numOptimizationCalls += 1 + errorAnalyzer->nodeNumOptCallsMap[node];


        logging->debug("Output Extrema: ", errorAnalysisResults[node].outputExtrema);
    }

    logging->debug("Output extremas found!");
}

void Graph::findErrorExtrema(const std::set<ir::Node*>& candidate_nodes) {
    logging->debug("Finding error extrema...");

    setupDerivativeComputation(candidate_nodes);

    errorAnalyzer->derivativeComputingDriver();

    if (concretizeErrorComps) concretizeErrorComponents();

    errorAnalyzer->errorComputingDriver(candidate_nodes, ibexInterface);

    if (collectErrorCompData) examineBwdDerivativeAndLocalError();

    logging->debug("Solving for ", candidate_nodes.size(), " nodes");


    std::map<ir::Node*, OptResult> max;
    for (const auto& node : candidate_nodes) {
        logging->debug("Finding max for: ", node->id);

        logging->debug("Error Expression computed for node ", node->id);

        max[node] = ibexInterface->findAbsMax(*errorAnalyzer->errAccumulator[node]);

        // print the error interval - Max have to be flipped since we find the min of the negative
        // of the function
        logging->debug("Max Interval: ", -max[node].result);
    }

    for (const auto& node : candidate_nodes) {
        logging->debug("Error Extrema computed for node ", node->id);

        errorAnalysisResults[node].errorExtrema = ibex::Interval(-(max[node].result) * pow(2, -53));

        errorAnalysisResults[node].optPoint = max[node].optimumPoint;
        errorAnalysisResults[node].totalOptimizationTime += max[node].optimizationTime;
        errorAnalysisResults[node].numOptimizationCalls += 1;


        logging->debug("Error Extrema: ", errorAnalysisResults[node].errorExtrema, " at ",
                       errorAnalysisResults[node].optPoint);
    }

    logging->debug("Error extremas found!");
}

std::map<ir::Node*, ErrorAnalysisResult> Graph::simplifyWithAbstraction(const std::set<ir::Node*>& candidate_nodes,
                                                                        unsigned int max_depth, bool isFinal) {
    logging->debug("Final computation...");


    ibexInterface->setInputIntervals(inputs);
    generateIbexSymbols();
    ibexInterface->setVariables(inputs, symbolTables[currentScope]->table);
    generateExprDriver(candidate_nodes);

    findErrorExtrema(candidate_nodes);
    findOutputExtrema(candidate_nodes);

    if (isFinal) {
        logging->debug("Final Computation complete!");

        return errorAnalysisResults;
    }

    logging->debug("Abstracting nodes");
    for (const auto& node : candidate_nodes) logging->debug("    ", node->id);

    std::map<ir::Node*, std::vector<ibex::Interval>> results;

    for (const auto& node : candidate_nodes) {
        results[node].push_back(errorAnalysisResults[node].outputExtrema);
        results[node].push_back(errorAnalysisResults[node].errorExtrema * pow(2, +53));
    }

    abstractNodes(results);
    rebuildAst();

    return errorAnalysisResults;
}

/*
 * Modifies the probe list to include nodes that are common to all nodes in the probe list
 *
 * @param probeList List of nodes to modify
 *
 * @return A list of nodes that are common to all nodes in the probe list
 */
std::vector<ir::Node*> Graph::modProbeList() {
    std::vector<ir::Node*> probe_list;

    // Get nodes from symbol table corresponding to the output variables
    probe_list.reserve(outputs.size());
    for (const auto& output : outputs) probe_list.push_back(symbolTables[currentScope]->table[output]);

    return probe_list;
}

/*
 * Abstracts nodes in results
 *
 * @param results A map of nodes to their corresponding intervals
 */
void Graph::abstractNodes(std::map<ir::Node*, std::vector<ibex::Interval>> results) {
    logging->debug("Abstracting nodes...");


    // Turn node in results into VariableNodes and create corresponding FreeVariable nodes
    for (const auto& singleResult : results) {
        auto* node = singleResult.first;

        ir::VariableNode* converted_node;

        // Convert node to VariableNode
        converted_node = new ir::VariableNode(*node);
        converted_node->setAbsoluteError(&ibex::ExprConstant::new_scalar(singleResult.second[1].ub()));

        // Add converted node to nodes and symbol table
        nodes.insert(converted_node);
        symbolTables[currentScope]->table[converted_node->variable->name] = converted_node;

        // Create corresponding FreeVariable node using the singleResult IntervalVector
        auto* free_node = new ir::FreeVariable(*new ibex::Interval(singleResult.second[0]),
                                               ir::Node::RoundingType::FL64);
        inputs[converted_node->variable->name] = free_node;

        // Add free node to nodes and inputs
        nodes.insert(free_node);
        free_node->setAbsoluteError(&ibex::ExprConstant::new_scalar(singleResult.second[1].ub()));
        free_node->setRounding(converted_node->getRounding());


        logging->debug("Converted Node ", node->id, " --> variable (", converted_node->id, ")");

        logging->debug("Result for node ", node->id, " : Output: ", singleResult.second[0],
                       ", Error: ", singleResult.second[1],
                       ", Optimizer Calls: ", errorAnalysisResults[node].numOptimizationCalls);
    }

    logging->debug("Nodes abstracted!");
}

/*
 * Rebuilds the AST post abstraction
 */
void Graph::rebuildAst() {
    logging->debug("Rebuilding AST...");


    std::vector<ir::Node*> probe_list = modProbeList();

    std::map<ir::Node*, unsigned int> completed;

    // Recursively call RebuildASTNode on nodes in probe_list if not already completed
    for (const auto& node : probe_list) {
        if (completed.find(node) == completed.end()) rebuildAstNode(node, completed);
    }

    // Get max depth among nodes in probe_list
    int max_depth = -1;
    for (const auto& node : probe_list) max_depth = std::max(int(node->depth), max_depth);

    // Get total number of nodes before
    unsigned int num_nodes = 0;
    for (const auto& depth_table : depthTable) num_nodes += depth_table.second.size();

    // Print num_nodes before
    logging->debug("Num nodes before: ", num_nodes);


    // Modify depthTable using the completed map
    depthTable.clear();
    for (const auto& node : completed) depthTable[int(node.second)].insert(node.first);

    // Get total number of nodes after
    num_nodes = 0;
    for (const auto& depth_table : depthTable) num_nodes += depth_table.second.size();

    // Print num_nodes after
    logging->debug("Num nodes after: ", num_nodes);


    logging->debug("AST rebuilt!");
}

void Graph::rebuildAstNode(ir::Node* node, std::map<ir::Node*, unsigned int>& completed) {
    // Recursively call RebuildASTNode on children of node if not already completed
    switch (node->type) {
        case ir::Node::Type::INTEGER:
        case ir::Node::Type::FLOAT:
        case ir::Node::Type::DOUBLE:
        case ir::Node::Type::FREE_VARIABLE:
        case ir::Node::Type::VARIABLE:
            node->depth = 0;
            break;
        case ir::Node::Type::UNARY_OP:
            if ((dynamic_cast<ir::UnaryOp*>(node))->op != ir::Node::OpType::NEG) {
                if (completed.find((dynamic_cast<ir::UnaryOp*>(node))->operand) == completed.end()) {
                    rebuildAstNode((dynamic_cast<ir::UnaryOp*>(node))->operand, completed);
                }

                node->depth = (dynamic_cast<ir::UnaryOp*>(node))->operand->depth + 1;
                completed[node] = node->depth;
            }
            break;
        case ir::Node::Type::BINARY_OP:
            if (completed.find((dynamic_cast<ir::BinaryOp*>(node))->leftOperand) == completed.end()) {
                rebuildAstNode((dynamic_cast<ir::BinaryOp*>(node))->leftOperand, completed);
            }
            if (completed.find((dynamic_cast<ir::BinaryOp*>(node))->rightOperand) == completed.end()) {
                rebuildAstNode((dynamic_cast<ir::BinaryOp*>(node))->rightOperand, completed);
            }
            node->depth = std::max((dynamic_cast<ir::BinaryOp*>(node))->leftOperand->depth,
                                   (dynamic_cast<ir::BinaryOp*>(node))->rightOperand->depth)
                        + 1;
            completed[node] = node->depth;
            break;
        case ir::Node::Type::TERNARY_OP:
            if (completed.find((dynamic_cast<ir::TernaryOp*>(node))->leftOperand) == completed.end()) {
                rebuildAstNode((dynamic_cast<ir::TernaryOp*>(node))->leftOperand, completed);
            }
            if (completed.find((dynamic_cast<ir::TernaryOp*>(node))->middleOperand) == completed.end()) {
                rebuildAstNode((dynamic_cast<ir::TernaryOp*>(node))->middleOperand, completed);
            }
            if (completed.find((dynamic_cast<ir::TernaryOp*>(node))->rightOperand) == completed.end()) {
                rebuildAstNode((dynamic_cast<ir::TernaryOp*>(node))->rightOperand, completed);
            }
            node->depth = std::max({(dynamic_cast<ir::TernaryOp*>(node))->leftOperand->depth,
                                    (dynamic_cast<ir::TernaryOp*>(node))->middleOperand->depth,
                                    (dynamic_cast<ir::TernaryOp*>(node))->rightOperand->depth})
                        + 1;
            completed[node] = node->depth;
            break;
        default:
            logging->debug("Unknown node type");
            break;
    }

    // Modify node
    if ((node->isUnaryOp() && (dynamic_cast<ir::UnaryOp*>(node))->op != ir::Node::OpType::NEG) || node->isBinaryOp()
        || node->isTernaryOp()) {
        completed[node] = node->depth;
    } else {
        node->depth = 0;
    }
}

int Graph::parse(const char& f) {
    yydebug = 0;
    yyin = fopen(&f, "r");
    if (yyin == nullptr) {
        logging->debug("Bad Input. Non-existant file");
        return -1;
    }

    do {
        logging->debug("Parsing...");

        createNewSymbolTable();
        if (yyparse(this) != 0) {
            logging->debug("Parsing failed");
            return 1;
        }
        logging->debug("Parsing successful!");

    } while (feof(yyin) == 0);

    return 0;
}
