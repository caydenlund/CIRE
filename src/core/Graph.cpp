#include "cire/core/Graph.h"
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

void Graph::registerLLVMNode(llvm::Value* llvmValue, Node* node) {
    if (llvmValue && node) {
        llvmValueToNode[llvmValue] = node;
        nodeToLLVMValue[node] = llvmValue;
    }
}

Node* Graph::getNodeByLLVMValue(llvm::Value* llvmValue) const {
    auto it = llvmValueToNode.find(llvmValue);
    return (it != llvmValueToNode.end()) ? it->second : nullptr;
}

llvm::Value* Graph::getLLVMValueByNode(Node* node) const {
    auto it = nodeToLLVMValue.find(node);
    return (it != nodeToLLVMValue.end()) ? it->second : nullptr;
}

std::vector<Node*> Graph::getNodesByInstructionType(const std::string& type) const {
    auto it = instructionTypeIndex.find(type);
    return (it != instructionTypeIndex.end()) ? it->second : std::vector<Node*>();
}

void Graph::indexNodeByInstructionType(Node* node, const std::string& type) {
    if (node) {
        instructionTypeIndex[type].push_back(node);
    }
}

std::ostream& operator<<(std::ostream& os, const Graph& graph) {
    graph.write(os);
    return os;
}

void Graph::write(std::ostream& out) const {
    out << "Graph:\n";
    out << "Inputs:\n";
    for (const auto& input : inputs) { out << "    " << input.first << " : " << *input.second; }
    out << "Outputs:\n";
    for (const auto& output : outputs) { out << "    " << output; }
    out << "Variables:\n";
    for (const auto& variable : symbolTables.find(currentScope)->second->table) {
        out << "\t" << variable.first << " : " << *variable.second;
    }

    out << "Nodes:\n";
    for (const auto& node : nodes) { out << "    " << *node << "\n"; }

    out << "Depth Table:";
    for (const auto& depth : depthTable) {
        out << "    " << depth.first << " : ";
        for (const auto& node : depth.second) { out << *node << " "; }
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
        (dynamic_cast<VariableNode*>(symbolTables[currentScope]->table[input.first]))->variable = &(
                ibex::ExprSymbol::new_(input.first.c_str()));
        symbolTables[currentScope]->table[input.first]->setAbsoluteError(
                &ibex::ExprConstant::new_scalar(input.second->var->ub() * pow(2, -53)));
    }


    for (const auto& node : nodes) {
        switch (node->type) {
            case NodeType::INTEGER: {
                (dynamic_cast<Integer*>(node))->value = &(
                        ibex::ExprConstant::new_scalar((dynamic_cast<Integer*>(node))->val));
                node->setAbsoluteError(&ibex::ExprConstant::new_scalar(0.0));
                break;
            }
            case NodeType::FLOAT: {
                (dynamic_cast<Float*>(node))->value = &(
                        ibex::ExprConstant::new_scalar((dynamic_cast<Float*>(node))->val));
                node->setAbsoluteError(
                        &ibex::ExprConstant::new_scalar((dynamic_cast<Float*>(node))->val * pow(2, -24)));
                break;
            }
            case NodeType::DOUBLE: {
                (dynamic_cast<Double*>(node))->value = &(
                        ibex::ExprConstant::new_scalar((dynamic_cast<Double*>(node))->val));
                node->setAbsoluteError(
                        &ibex::ExprConstant::new_scalar((dynamic_cast<Double*>(node))->val * pow(2, -53)));
                break;
            }
            case NodeType::FREE_VARIABLE: {
                node->setAbsoluteError(&ibex::ExprConstant::new_scalar(((FreeVariable*)node)->var->ub() * pow(2, -53)));
                break;
            }
            case NodeType::VARIABLE:  // The absoluteError has already been set in the previous
                                      // inputs for loop
            // Following nodes do not have an absolute error. Only Constants and FreeVariables have
            // an absolute error
            case NodeType::UNARY_OP:
            case NodeType::BINARY_OP:
            case NodeType::TERNARY_OP:
            case NodeType::DEFAULT: {
                break;
            }
        }
    }
}

Node* Graph::findFreeVarNode(string Var) const {
    auto it = inputs.find(Var);
    if (it != inputs.end()) { return it->second; }

    return nullptr;
}

Node* Graph::findVarNode(string Var) const {
    auto it = symbolTables.find(currentScope)->second->table.find(Var);
    if (it != symbolTables.find(currentScope)->second->table.end()) { return it->second; }

    return nullptr;
}

void Graph::setupDerivativeComputation(std::set<Node*> candidate_nodes) {
    // Set up output
    // Get the max depth of the candidate_nodes
    unsigned int max_depth = 0;
    for (const auto& node : candidate_nodes) { max_depth = std::max<unsigned int>(node->depth, max_depth); }

    errorAnalyzer->derivativeComputedNodes.clear();
    errorAnalyzer->errorComputedNodes.clear();
    errorAnalyzer->numParentsOfNode.clear();
    errorAnalyzer->parentsOfNode.clear();
    errorAnalyzer->bwdDerivatives.clear();
    errorAnalyzer->typeCastRnd.clear();
    errorAnalyzer->errAccumulator.clear();

    // Insert candidate_nodes with max depth into worklist
    for (const auto& node : candidate_nodes) {
        if (node->depth == max_depth) { errorAnalyzer->workList.insert(node); }
    }

    // Set BwdDerivatives of each candidate_node (output node) with respect to itself to 1
    for (const auto& node : candidate_nodes) {
        errorAnalyzer->bwdDerivatives[node][node] = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(1);
        errorAnalyzer->typeCastRnd[node][node] = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(0);
    }

    // Set numParentsOfNode of each node to the number of parents it has
    for (const auto& node : candidate_nodes) { errorAnalyzer->numParentsOfNode[node] = node->parents.size(); }
}

// Generates Expressions corresponding to all candidate_nodes bottom up
void Graph::generateExprDriver(const std::set<Node*>& candidate_nodes) {
    // Map from depth to nodes at that depth whose expression has been generated. Similar to
    // "reachable" in Satire
    std::map<int, std::set<Node*>> generatedExprsAtDepth;

    // Map from Ibex::ExprNode to the Nodes that have that expression
    // Common Subexpression Elimination Table
    // This keeps track of nodes that have the same expression and can be replaced by a single node
    std::map<ibex::ExprNode*, std::set<Node*>> cseTable;

    logging->info("Generating expressions...");

    for (const auto& node : candidate_nodes) {
        logging->debug("Processing node '", node->id, "'");
        if (generatedExprsAtDepth[node->depth].find(node) == generatedExprsAtDepth[node->depth].end()) {
            generateExpr(node, generatedExprsAtDepth, cseTable);
        }
        logging->debug("Processed node '", node->id, "'");
    }

    logging->info("Done generating expressions");
}

void Graph::generateExpr(Node* node, std::map<int, std::set<Node*>>& generatedExprsAtDepth,
                         std::map<ibex::ExprNode*, std::set<Node*>>& cseTable) {
    switch (node->type) {
        case NodeType::INTEGER:
        case NodeType::FLOAT:
        case NodeType::DOUBLE:
        case NodeType::FREE_VARIABLE:
        case NodeType::VARIABLE:
            // Already has an expression or interval
            break;
        case NodeType::UNARY_OP:
            if (generatedExprsAtDepth[((UnaryOp*)node)->operand->depth].find(((UnaryOp*)node)->operand)
                == generatedExprsAtDepth[((UnaryOp*)node)->operand->depth].end()) {
                generateExpr(((UnaryOp*)node)->operand, generatedExprsAtDepth, cseTable);
            }
            ((UnaryOp*)node)->expr = (ibex::ExprUnaryOp*)&node->generateSymExpr();
            errorAnalyzer->parentsOfNode[((UnaryOp*)node)->operand].insert(node);
            logging->debug("Node '", node->id, "' processed");
            logging->debug("    UnaryOp processed");
            break;
        case NodeType::BINARY_OP:
            if (generatedExprsAtDepth[((BinaryOp*)node)->leftOperand->depth].find(((BinaryOp*)node)->leftOperand)
                == generatedExprsAtDepth[((BinaryOp*)node)->leftOperand->depth].end()) {
                generateExpr(((BinaryOp*)node)->leftOperand, generatedExprsAtDepth, cseTable);
            }
            if (generatedExprsAtDepth[((BinaryOp*)node)->rightOperand->depth].find(((BinaryOp*)node)->rightOperand)
                == generatedExprsAtDepth[((BinaryOp*)node)->rightOperand->depth].end()) {
                generateExpr(((BinaryOp*)node)->rightOperand, generatedExprsAtDepth, cseTable);
            }
            ((BinaryOp*)node)->expr = (ibex::ExprBinaryOp*)&node->generateSymExpr();
            errorAnalyzer->parentsOfNode[((BinaryOp*)node)->leftOperand].insert(node);
            errorAnalyzer->parentsOfNode[((BinaryOp*)node)->rightOperand].insert(node);
            logging->debug("Node '", node->id, "' processed");
            logging->debug("    BinaryOp processed");
            break;
        case NodeType::TERNARY_OP:
            if (generatedExprsAtDepth[((TernaryOp*)node)->leftOperand->depth].find(((TernaryOp*)node)->leftOperand)
                == generatedExprsAtDepth[((TernaryOp*)node)->leftOperand->depth].end()) {
                generateExpr(((TernaryOp*)node)->leftOperand, generatedExprsAtDepth, cseTable);
            }
            if (generatedExprsAtDepth[((TernaryOp*)node)->middleOperand->depth].find(((TernaryOp*)node)->middleOperand)
                == generatedExprsAtDepth[((TernaryOp*)node)->middleOperand->depth].end()) {
                generateExpr(((TernaryOp*)node)->middleOperand, generatedExprsAtDepth, cseTable);
            }
            if (generatedExprsAtDepth[((TernaryOp*)node)->rightOperand->depth].find(((TernaryOp*)node)->rightOperand)
                == generatedExprsAtDepth[((TernaryOp*)node)->rightOperand->depth].end()) {
                generateExpr(((TernaryOp*)node)->rightOperand, generatedExprsAtDepth, cseTable);
            }
            // Ibex does not have a TernaryOp, so we split the Op into two BinaryOps
            ((TernaryOp*)node)->expr = (ibex::ExprBinaryOp*)&node->generateSymExpr();
            errorAnalyzer->parentsOfNode[((TernaryOp*)node)->leftOperand].insert(node);
            errorAnalyzer->parentsOfNode[((TernaryOp*)node)->middleOperand].insert(node);
            errorAnalyzer->parentsOfNode[((TernaryOp*)node)->rightOperand].insert(node);
            logging->debug("Node '", node->id, "' processed");
            logging->debug("    TernaryOp processed");
            break;
        default:
            logging->critical("Unknown node type");
    }

    // Update the map tracking processed nodes
    generatedExprsAtDepth[node->depth].insert(node);

    // Common sub expression elimination phase
    // 1) Find all subexpressions similar to the current node
    std::vector<Node*> nodes_to_merge;
    for (const auto& n : cseTable[node->getExprNode()]) {
        // Ensuring n and node are not the same nodes
        if (n != node) {
            // Check if all children of n and node are the same
            switch (n->type) {
                case NodeType::INTEGER:
                    if (node->isInteger()) nodes_to_merge.push_back(n);
                    break;
                case NodeType::FLOAT:
                    if (node->isFloat()) nodes_to_merge.push_back(n);
                    break;
                case NodeType::DOUBLE:
                    if (node->isDouble()) nodes_to_merge.push_back(n);
                    break;
                case NodeType::FREE_VARIABLE:
                    if (node->isFreeVariable()) nodes_to_merge.push_back(n);
                    break;
                case NodeType::VARIABLE:
                    if (node->isVariable()) nodes_to_merge.push_back(n);
                    break;
                case NodeType::UNARY_OP:
                    if (node->isUnaryOp() && ((UnaryOp*)n)->operand == ((UnaryOp*)node)->operand) {
                        nodes_to_merge.push_back(n);
                    }
                    break;
                case NodeType::BINARY_OP:
                    if (node->isBinaryOp() && ((BinaryOp*)n)->leftOperand == ((BinaryOp*)node)->leftOperand
                        && ((BinaryOp*)n)->rightOperand == ((BinaryOp*)node)->rightOperand) {
                        nodes_to_merge.push_back(n);
                    }
                    break;
                case NodeType::TERNARY_OP:
                    if (node->isTernaryOp() && ((TernaryOp*)n)->leftOperand == ((TernaryOp*)node)->leftOperand
                        && ((TernaryOp*)n)->middleOperand == ((TernaryOp*)node)->middleOperand
                        && ((TernaryOp*)n)->rightOperand == ((TernaryOp*)node)->rightOperand) {
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
Node* Graph::mergeNodes(Node* node1, Node* node2, std::map<Node*, std::set<Node*>>& parentsOfNode) {
    // Set union of parents of node1 and node2
    std::set<Node*> new_parents;
    std::set_union(parentsOfNode[node1].begin(), parentsOfNode[node1].end(), parentsOfNode[node2].begin(),
                   parentsOfNode[node2].end(), std::inserter(new_parents, new_parents.end()));

    // Update the children of the new_parents set to point to node2 of they point to node1
    for (const auto& par : new_parents) {
        switch (par->type) {
            case NodeType::INTEGER:
            case NodeType::FLOAT:
            case NodeType::DOUBLE:
            case NodeType::FREE_VARIABLE:
            case NodeType::VARIABLE:
                break;
            case NodeType::UNARY_OP:
                if (((UnaryOp*)par)->operand == node1) { ((UnaryOp*)par)->operand = node2; }
                break;
            case NodeType::BINARY_OP:
                if (((BinaryOp*)par)->leftOperand == node1) { ((BinaryOp*)par)->leftOperand = node2; }
                if (((BinaryOp*)par)->rightOperand == node1) { ((BinaryOp*)par)->rightOperand = node2; }
                break;
            case NodeType::TERNARY_OP:
                if (((TernaryOp*)par)->leftOperand == node1) { ((TernaryOp*)par)->leftOperand = node2; }
                if (((TernaryOp*)par)->middleOperand == node1) { ((TernaryOp*)par)->middleOperand = node2; }
                if (((TernaryOp*)par)->rightOperand == node1) { ((TernaryOp*)par)->rightOperand = node2; }
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
        case NodeType::INTEGER:
        case NodeType::FLOAT:
        case NodeType::DOUBLE:
        case NodeType::FREE_VARIABLE:
        case NodeType::VARIABLE:
        case NodeType::DEFAULT:
            break;
        case NodeType::UNARY_OP:
            parentsOfNode[((UnaryOp*)node2)->operand].erase(node1);
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
    int totalNodesInBwdDerivatives = errorAnalyzer->bwdDerivatives.size();
    int processedNodes = 0;

    // Iterate through the bwdDerivative map
    for (const auto& node_bwd_derivatives : errorAnalyzer->bwdDerivatives) {
        Node* node = node_bwd_derivatives.first;

        node_bwd_derivatives.second.size();

        logging->debug("Processing Node '", node->id, "'. Processed ", processedNodes, "/", totalNodesInBwdDerivatives);

        // Iterate through the nodeBwdDerivatives map
        for (const auto& node_output_bwd_derivative : node_bwd_derivatives.second) {
            Node* output_node = node_output_bwd_derivative.first;
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
    std::map<Node*, std::map<Node*, std::pair<double, double>>> evaluatedBwdDerivatives;

    // Iterate through the bwdDerivative map
    for (const auto& node_bwd_derivatives : errorAnalyzer->bwdDerivatives) {
        Node* node = node_bwd_derivatives.first;
        // Iterate through the nodeBwdDerivatives map
        for (const auto& node_output_bwd_derivative : node_bwd_derivatives.second) {
            Node* output_node = node_output_bwd_derivative.first;
            OptResult max_bwd = ibexInterface->findAbsMax(*node_output_bwd_derivative.second);
            OptResult max_local_err = ibexInterface->findAbsMax(
                    const_cast<ibex::ExprNode&>(product(node->getAbsoluteError(), node->getRounding())));

            evaluatedBwdDerivatives[node][output_node] = std::make_pair((-max_bwd.result).mag(),
                                                                        (-max_local_err.result).mag());
        }
    }

    // print the evaluatedBwdDerivatives
    for (const auto& node_bwd_derivatives : evaluatedBwdDerivatives) {
        Node* node = node_bwd_derivatives.first;
        for (const auto& node_output_bwd_derivative : node_bwd_derivatives.second) {
            Node* output_node = node_output_bwd_derivative.first;
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
        Node* node = node_bwd_derivatives.first;
        for (const auto& node_output_bwd_derivative : node_bwd_derivatives.second) {
            Node* output_node = node_output_bwd_derivative.first;
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
                return compareDAGs(((ibex::ExprAdd*)&expr1)->left, ((ibex::ExprAdd*)&expr2)->left)
                    && compareDAGs(((ibex::ExprAdd*)&expr1)->right, ((ibex::ExprAdd*)&expr2)->right);
            case ibex::ExprNode::NumExprMul:
                return compareDAGs(((ibex::ExprMul*)&expr1)->left, ((ibex::ExprMul*)&expr2)->left)
                    && compareDAGs(((ibex::ExprMul*)&expr1)->right, ((ibex::ExprMul*)&expr2)->right);
            case ibex::ExprNode::NumExprSub:
                return compareDAGs(((ibex::ExprSub*)&expr1)->left, ((ibex::ExprSub*)&expr2)->left)
                    && compareDAGs(((ibex::ExprSub*)&expr1)->right, ((ibex::ExprSub*)&expr2)->right);
            case ibex::ExprNode::NumExprDiv:
                return compareDAGs(((ibex::ExprDiv*)&expr1)->left, ((ibex::ExprDiv*)&expr2)->left)
                    && compareDAGs(((ibex::ExprDiv*)&expr1)->right, ((ibex::ExprDiv*)&expr2)->right);
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
std::set<Node*> Graph::flattenSubDags(Node* node, unsigned int min_depth, unsigned int max_depth) {
    assert(min_depth <= max_depth && "Invalid bounds for flattening");

    std::set<Node*> nodes_to_flatten;

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
            if (((UnaryOp*)node)->operand->depth >= min_depth && ((UnaryOp*)node)->operand->depth <= max_depth) {
                nodes_to_flatten.insert(((UnaryOp*)node)->operand);
            }
            if (((UnaryOp*)node)->operand->depth > min_depth) {
                std::set_union(nodes_to_flatten.begin(), nodes_to_flatten.end(),
                               flattenSubDags(((UnaryOp*)node)->operand, min_depth, max_depth).begin(),
                               flattenSubDags(((UnaryOp*)node)->operand, min_depth, max_depth).end(),
                               std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
            }
            break;
        case NodeType::BINARY_OP:
            if (((BinaryOp*)node)->leftOperand->depth >= min_depth
                && ((BinaryOp*)node)->leftOperand->depth <= max_depth) {
                nodes_to_flatten.insert(((BinaryOp*)node)->leftOperand);
            }
            if (((BinaryOp*)node)->rightOperand->depth >= min_depth
                && ((BinaryOp*)node)->rightOperand->depth <= max_depth) {
                nodes_to_flatten.insert(((BinaryOp*)node)->rightOperand);
            }
            if (((BinaryOp*)node)->leftOperand->depth > min_depth) {
                std::set_union(nodes_to_flatten.begin(), nodes_to_flatten.end(),
                               flattenSubDags(((BinaryOp*)node)->leftOperand, min_depth, max_depth).begin(),
                               flattenSubDags(((BinaryOp*)node)->leftOperand, min_depth, max_depth).end(),
                               std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
            }
            if (((BinaryOp*)node)->rightOperand->depth > min_depth) {
                std::set_union(nodes_to_flatten.begin(), nodes_to_flatten.end(),
                               flattenSubDags(((BinaryOp*)node)->rightOperand, min_depth, max_depth).begin(),
                               flattenSubDags(((BinaryOp*)node)->rightOperand, min_depth, max_depth).end(),
                               std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
            }
            break;
        case NodeType::TERNARY_OP:
            if (((TernaryOp*)node)->leftOperand->depth >= min_depth
                && ((TernaryOp*)node)->leftOperand->depth <= max_depth) {
                nodes_to_flatten.insert(((TernaryOp*)node)->leftOperand);
            }
            if (((TernaryOp*)node)->middleOperand->depth >= min_depth
                && ((TernaryOp*)node)->middleOperand->depth <= max_depth) {
                nodes_to_flatten.insert(((TernaryOp*)node)->middleOperand);
            }
            if (((TernaryOp*)node)->rightOperand->depth >= min_depth
                && ((TernaryOp*)node)->rightOperand->depth <= max_depth) {
                nodes_to_flatten.insert(((TernaryOp*)node)->rightOperand);
            }
            if (((TernaryOp*)node)->leftOperand->depth > min_depth) {
                std::set_union(nodes_to_flatten.begin(), nodes_to_flatten.end(),
                               flattenSubDags(((TernaryOp*)node)->leftOperand, min_depth, max_depth).begin(),
                               flattenSubDags(((TernaryOp*)node)->leftOperand, min_depth, max_depth).end(),
                               std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
            }
            if (((TernaryOp*)node)->middleOperand->depth > min_depth) {
                std::set_union(nodes_to_flatten.begin(), nodes_to_flatten.end(),
                               flattenSubDags(((TernaryOp*)node)->middleOperand, min_depth, max_depth).begin(),
                               flattenSubDags(((TernaryOp*)node)->middleOperand, min_depth, max_depth).end(),
                               std::inserter(nodes_to_flatten, nodes_to_flatten.end()));
            }
            if (((TernaryOp*)node)->rightOperand->depth > min_depth) {
                std::set_union(nodes_to_flatten.begin(), nodes_to_flatten.end(),
                               flattenSubDags(((TernaryOp*)node)->rightOperand, min_depth, max_depth).begin(),
                               flattenSubDags(((TernaryOp*)node)->rightOperand, min_depth, max_depth).end(),
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
std::set<Node*> Graph::findCommonNodes(Node* node, unsigned int min_depth, unsigned int max_depth) {
    std::set<Node*> common_nodes;

    // Create a list of flattened subDAGs of children of node
    std::vector<std::set<Node*>> flattened_subDAGs;
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
            flattened_subDAGs.push_back(flattenSubDags(((UnaryOp*)node)->operand, min_depth, max_depth));
            break;
        case NodeType::BINARY_OP:
            flattened_subDAGs.push_back(flattenSubDags(((BinaryOp*)node)->leftOperand, min_depth, max_depth));
            flattened_subDAGs.push_back(flattenSubDags(((BinaryOp*)node)->rightOperand, min_depth, max_depth));
            break;
        case NodeType::TERNARY_OP:
            flattened_subDAGs.push_back(flattenSubDags(((TernaryOp*)node)->leftOperand, min_depth, max_depth));
            flattened_subDAGs.push_back(flattenSubDags(((TernaryOp*)node)->middleOperand, min_depth, max_depth));
            flattened_subDAGs.push_back(flattenSubDags(((TernaryOp*)node)->rightOperand, min_depth, max_depth));
            break;
        default:
            logging->critical("Unknown node type");
    }

    flattened_subDAGs.push_back(std::set<Node*>({node}));

    // Find common nodes
    for (const auto& flattened_subDAG : flattened_subDAGs) {
        if (common_nodes.empty()) {
            common_nodes = flattened_subDAG;
        } else {
            std::set<Node*> temp;
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
std::map<Node*, std::set<Node*>> Graph::findCommonDependencies(std::set<Node*> nodes, unsigned int min_depth,
                                                               unsigned int max_depth) {
    std::map<Node*, std::set<Node*>> common_dependencies;

    // Populate common_dependencies with common_nodes from each node's common node list
    for (const auto& node : nodes) {
        std::set<Node*> initial_dependence_list = findCommonNodes(node, min_depth, max_depth);
        std::vector<std::set<Node*>> common_nodes_list;

        // Populate common_nodes_list with common_nodes from each node's common node list
        for (const auto& node : initial_dependence_list) {
            common_nodes_list.push_back(findCommonNodes(node, min_depth, max_depth));
        }

        std::set<Node*> redundant_nodes;

        // Unionize common_nodes_list into redundant_nodes
        for (const auto& common_nodes : common_nodes_list) {
            std::set_union(redundant_nodes.begin(), redundant_nodes.end(), common_nodes.begin(), common_nodes.end(),
                           std::inserter(redundant_nodes, redundant_nodes.end()));
        }

        // Get the set difference between initial_dependence_list and redundant_nodes
        std::set<Node*> common_nodes;
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
std::set<Node*> Graph::filterNodesWithOperationWithinDepth(Node::Op op, unsigned int max_depth) {
    std::set<Node*> nodes_with_op;

    std::copy_if(nodes.begin(), nodes.end(), std::inserter(nodes_with_op, nodes_with_op.end()),
                 [op, max_depth](Node* node) {
                     return node->depth <= max_depth && (node->isUnaryOp() && ((UnaryOp*)node)->op == op)
                         || (node->isBinaryOp() && ((BinaryOp*)node)->op == op);
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
std::set<Node*> Graph::filterCandidatesForAbstraction(unsigned int max_depth, unsigned int lower_bound,
                                                      unsigned int upper_bound) {
    assert(lower_bound <= upper_bound && upper_bound <= max_depth && "Invalid bounds for abstraction");

    std::set<Node*> nodes_with_op = filterNodesWithOperationWithinDepth(Node::Op::DIV, max_depth);

    // Print nodes with op
    logging->debug("Nodes with op:");
    for (const auto& node : nodes_with_op) { logging->debug("    Node ID: ", node->id); }

    std::map<Node*, std::set<Node*>> common_dependencies = findCommonDependencies(nodes_with_op, lower_bound,
                                                                                  upper_bound);

    // Print common dependencies
    logging->debug("Common dependencies:");
    for (const auto& common_dependency : common_dependencies) {
        logging->debug("    Node ID: ", common_dependency.first->id, ":");
        for (const auto& node : common_dependency.second) { logging->debug("        Node ID: ", node->id); }
    }

    // Unionize the node set from common_dependencies
    std::set<Node*> common_dependencies_set;
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
        for (const auto& node : common_dependencies_set) {
            if (node->depth > local_max_depth) { local_max_depth = node->depth; }
        }

        // Get nodes from common_dependencies_set with depth equal to local_max_depth
        std::set<Node*> common_dependencies_set;
        for (const auto& node : common_dependencies_set) {
            if (node->depth == local_max_depth) { common_dependencies_set.insert(node); }
        }
    }

    // Print common dependencies set
    logging->debug("Common dependencies set:");
    for (const auto& node : common_dependencies_set) logging->debug("    Node ID: ", node->id);

    return common_dependencies_set;
}

std::pair<unsigned int, std::set<Node*>>
Graph::selectNodesForAbstraction(unsigned int max_depth, unsigned int bound_min_depth, unsigned int bound_max_depth) {
    assert(bound_min_depth <= bound_max_depth && bound_max_depth <= max_depth && "Invalid bounds for abstraction");
    std::set<Node*> nodes_to_abstract;

    logging->debug("Selecting nodes for abstraction...");

    // Abstraction window is just 1 level wide
    if (bound_min_depth == bound_max_depth && bound_max_depth <= max_depth) {
        return std::make_pair(bound_min_depth, depthTable[bound_min_depth]);
    }

    std::set<Node*> initialCandidateList = filterCandidatesForAbstraction(max_depth, bound_min_depth, bound_max_depth);

    unsigned int local_max_depth = bound_max_depth;


    // Keep increasing local_max_depth until initialCandidateList is not empty
    while (initialCandidateList.empty() && local_max_depth <= max_depth) {
        local_max_depth += 5;
        initialCandidateList = filterCandidatesForAbstraction(max_depth, bound_min_depth, local_max_depth);
    }

    if (initialCandidateList.empty()) {
        logging->debug("No candidates found!");
        return std::make_pair(-1, std::set<Node*>());
    } else {
        local_max_depth = 0;
        // Set local_max_depth to the greatest depth of nodes in initialCandidateList
        for (const auto& node : initialCandidateList) {
            if (node->depth > local_max_depth) { local_max_depth = node->depth; }
        }

        auto f = [&local_max_depth](Node* x) { return float(x->depth) / (local_max_depth + 0.01); };

        auto g = [](Node* x, auto y) { return (-1) * y * log2(y) * x->parents.size(); };

        // Create a list of cost
        std::map<Node*, double> cost_dict;

        // Compute g(x, f(x)) for each node in initialCandidateList
        for (const auto& node : initialCandidateList) { cost_dict[node] = g(node, f(node)); }

        // Sum cost of all nodes with same depth
        std::map<int, double> cost_sum_dict;
        for (const auto& node : initialCandidateList) { cost_sum_dict[node->depth] += cost_dict[node]; }

        // Print cost_sum_dict
        logging->debug("Cost Sum Dict:");
        for (const auto& cost_sum : cost_sum_dict) { logging->debug("\t", cost_sum.first, " : ", cost_sum.second); }

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
            for (const auto& node : candidate_nodes) { logging->debug("    Node ID: ", node->id); }
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
                if (op_count < max_operators_count) { max_operators_count = op_count; }
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

                if (bound_max_depth - bound_min_depth > 4) { bound_min_depth = bound_min_depth - 2; }
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

void Graph::findOutputExtrema(const std::set<Node*>& candidate_nodes) {
    logging->debug("Finding output extremas for ", candidate_nodes.size(), " nodes");


    std::map<Node*, OptResult> max;
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

void Graph::findErrorExtrema(const std::set<Node*>& candidate_nodes) {
    logging->debug("Finding error extrema...");

    setupDerivativeComputation(candidate_nodes);

    errorAnalyzer->derivativeComputingDriver();

    if (concretizeErrorComps) { concretizeErrorComponents(); }

    errorAnalyzer->errorComputingDriver(candidate_nodes, ibexInterface);

    if (collectErrorCompData) { examineBwdDerivativeAndLocalError(); }

    logging->debug("Solving for ", candidate_nodes.size(), " nodes");


    std::map<Node*, OptResult> max;
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

std::map<Node*, ErrorAnalysisResult> Graph::simplifyWithAbstraction(const std::set<Node*>& candidate_nodes,
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
    for (const auto& node : candidate_nodes) { logging->debug("    ", node->id); }

    std::map<Node*, std::vector<ibex::Interval>> results;

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
std::vector<Node*> Graph::modProbeList() {
    std::vector<Node*> probe_list;

    // Get nodes from symbol table corresponding to the output variables
    for (const auto& output : outputs) { probe_list.push_back(symbolTables[currentScope]->table[output]); }

    return probe_list;
}

/*
 * Abstracts nodes in results
 *
 * @param results A map of nodes to their corresponding intervals
 */
void Graph::abstractNodes(std::map<Node*, std::vector<ibex::Interval>> results) {
    logging->debug("Abstracting nodes...");


    // Turn node in results into VariableNodes and create corresponding FreeVariable nodes
    for (const auto& singleResult : results) {
        auto* node = singleResult.first;

        VariableNode* converted_node;

        // Convert node to VariableNode
        converted_node = new VariableNode(*node);
        converted_node->setAbsoluteError(&ibex::ExprConstant::new_scalar(singleResult.second[1].ub()));

        // Add converted node to nodes and symbol table
        nodes.insert(converted_node);
        symbolTables[currentScope]->table[converted_node->variable->name] = converted_node;

        // Create corresponding FreeVariable node using the singleResult IntervalVector
        auto* free_node = new FreeVariable(*new ibex::Interval(singleResult.second[0]), Node::RoundingType::FL64);
        inputs[converted_node->variable->name] = free_node;

        // Add free node to nodes and inputs
        nodes.insert(free_node);
        free_node->setAbsoluteError(&ibex::ExprConstant::new_scalar(singleResult.second[1].ub()));
        free_node->setRounding(converted_node->getRounding());


        logging->debug("Converted Node ", node->id, " --> variable (", converted_node->id, ")");

        logging->debug("Result for node ", node->id, " : Output: ", singleResult.second[0], ", Error: ", singleResult.second[1], ", Optimizer Calls: ", errorAnalysisResults[node].numOptimizationCalls);
    }

    logging->debug("Nodes abstracted!");
}

/*
 * Rebuilds the AST post abstraction
 */
void Graph::rebuildAst() {
    logging->debug("Rebuilding AST...");


    std::vector<Node*> probe_list = modProbeList();

    std::map<Node*, unsigned int> completed;

    // Recursively call RebuildASTNode on nodes in probe_list if not already completed
    for (const auto& node : probe_list) {
        if (completed.find(node) == completed.end()) rebuildAstNode(node, completed);
    }

    // Get max depth among nodes in probe_list
    int max_depth = -1;
    for (const auto& node : probe_list) max_depth = std::max(node->depth, max_depth);

    // Get total number of nodes before
    unsigned int num_nodes = 0;
    for (const auto& depth_table : depthTable) { num_nodes += depth_table.second.size(); }

    // Print num_nodes before
    logging->debug("Num nodes before: ", num_nodes);


    // Modify depthTable using the completed map
    depthTable.clear();
    for (const auto& node : completed) { depthTable[node.second].insert(node.first); }

    // Get total number of nodes after
    num_nodes = 0;
    for (const auto& depth_table : depthTable) { num_nodes += depth_table.second.size(); }

    // Print num_nodes after
    logging->debug("Num nodes after: ", num_nodes);


    logging->debug("AST rebuilt!");
}

void Graph::rebuildAstNode(Node* node, std::map<Node*, unsigned int>& completed) {
    // Recursively call RebuildASTNode on children of node if not already completed
    switch (node->type) {
        case NodeType::INTEGER:
        case NodeType::FLOAT:
        case NodeType::DOUBLE:
        case NodeType::FREE_VARIABLE:
        case NodeType::VARIABLE:
            node->depth = 0;
            break;
        case NodeType::UNARY_OP:
            if (((UnaryOp*)node)->op != Node::Op::NEG) {
                if (completed.find(((UnaryOp*)node)->operand) == completed.end()) {
                    rebuildAstNode(((UnaryOp*)node)->operand, completed);
                }

                node->depth = ((UnaryOp*)node)->operand->depth + 1;
                completed[node] = node->depth;
            }
            break;
        case NodeType::BINARY_OP:
            if (completed.find(((BinaryOp*)node)->leftOperand) == completed.end()) {
                rebuildAstNode(((BinaryOp*)node)->leftOperand, completed);
            }
            if (completed.find(((BinaryOp*)node)->rightOperand) == completed.end()) {
                rebuildAstNode(((BinaryOp*)node)->rightOperand, completed);
            }
            node->depth = std::max(((BinaryOp*)node)->leftOperand->depth, ((BinaryOp*)node)->rightOperand->depth) + 1;
            completed[node] = node->depth;
            break;
        case NodeType::TERNARY_OP:
            if (completed.find(((TernaryOp*)node)->leftOperand) == completed.end()) {
                rebuildAstNode(((TernaryOp*)node)->leftOperand, completed);
            }
            if (completed.find(((TernaryOp*)node)->middleOperand) == completed.end()) {
                rebuildAstNode(((TernaryOp*)node)->middleOperand, completed);
            }
            if (completed.find(((TernaryOp*)node)->rightOperand) == completed.end()) {
                rebuildAstNode(((TernaryOp*)node)->rightOperand, completed);
            }
            node->depth = std::max({((TernaryOp*)node)->leftOperand->depth, ((TernaryOp*)node)->middleOperand->depth,
                                    ((TernaryOp*)node)->rightOperand->depth})
                        + 1;
            completed[node] = node->depth;
            break;
        default:
            logging->debug("Unknown node type");
            break;
    }

    // Modify node
    if ((node->isUnaryOp() && ((UnaryOp*)node)->op != Node::Op::NEG) || node->isBinaryOp() || node->isTernaryOp()) {
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
