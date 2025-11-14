#include "cire/core/ErrorAnalyzer.h"
#include "cire/core/Results.h"
#include "cire/core/Graph.h"
#include "cire/interfaces/Logging.h"

#include <algorithm>

std::string getOpString(Node::Op op) {
    switch (op) {
        case Node::ADD: return "ADD";
        case Node::SUB: return "SUB";
        case Node::MUL: return "MUL";
        case Node::DIV: return "DIV";
        case Node::NEG: return "NEG";
        case Node::SIN: return "SIN";
        case Node::COS: return "COS";
        case Node::TAN: return "TAN";
        case Node::SINH: return "SINH";
        case Node::COSH: return "COSH";
        case Node::TANH: return "TANH";
        case Node::ASIN: return "ASIN";
        case Node::ACOS: return "ACOS";
        case Node::ATAN: return "ATAN";
        case Node::LOG: return "LOG";
        case Node::SQRT: return "SQRT";
        case Node::EXP: return "EXP";
        case Node::FMA: return "FMA";
        case Node::FPTRUNC: return "FPTRUNC";
        case Node::FPEXT: return "FPEXT";
        default: return "UNKNOWN";
    }
}

ErrorAnalyzer::ErrorAnalyzer() = default;

bool ErrorAnalyzer::parentsVisited(Node* node) { return numParentsOfNode[node] >= parentsOfNode[node].size(); }

void ErrorAnalyzer::derivativeComputingDriver() {
    if (logging) { logging->debug("Computing Derivatives..."); }

    int next_depth = -1;

    // Iterate all nodes in the worklist
    while (!workList.empty()) {
        Node* node = *workList.begin();
        workList.erase(node);

        int current_depth = node->depth;
        next_depth = current_depth - 1;
        // If node contains a constant, add it to completed list as you cannot
        // compute its derivative.
        if (derivativeComputedNodes[current_depth].find(node) != derivativeComputedNodes[current_depth].end()) {
            // If derivative of node has already been computed, move on.
        }
        // These are constants and their derivatives are 0
        else if (node->type == NodeType::INTEGER || node->type == NodeType::FLOAT || node->type == NodeType::DOUBLE) {
            derivativeComputedNodes[current_depth].insert(node);
        }
        // If all parents of node have been visited, compute derivative of node
        else if (parentsVisited(node)) {
            derivativeComputing(node);
        }
        // This means that requirements for computing derivative of node are not met
        //  and we need to add it to the worklist again
        // Requirements are that all parents of node have been visited and node is not a constant
        else {
            workList.insert(node);
        }

        // If all nodes at current depth have been processed, move to next depth and add nodes at
        // that depth to worklist
        if (workList.empty() && !nextWorkList.empty() && next_depth >= 0) {
            std::copy_if(nextWorkList.begin(), nextWorkList.end(), std::inserter(workList, workList.end()),
                         [&next_depth](Node* node) { return node->depth == next_depth; });
        }
    }

    if (logging && logging->level <= LogLevel::DEBUG) {
        printBwdDerivativesIbexExprs();
        std::cout << std::endl;
    }

    if (logging) { logging->debug("Derivatives computed!"); }
}

// Compute the Backward derivative of outVar with respect to node's children by using the chain rule
void ErrorAnalyzer::derivativeComputing(Node* node) {
    // TODO: Type rounding handled only for FL64 --> FL32. Handle for other cases.
    std::vector<Node*> outputList = keys(bwdDerivatives[node]);
    for (Node* outVar : outputList) {
        assert(bwdDerivatives[node][outVar] != nullptr && "Derivative of output wrt node should have been computed\n");

        Node *operand, *leftOperand, *rightOperand;
        ibex::ExprNode *derivThroughNode, *derivLeftThroughNode, *derivRightThroughNode;
        ibex::ExprNode* typeCastRndVal;
        switch (node->type) {
            case DEFAULT:
            case INTEGER:
            case FLOAT:
            case DOUBLE:
            case FREE_VARIABLE:
            case VARIABLE:
                break;
            case UNARY_OP:
                operand = ((UnaryOp*)node)->operand;
                derivThroughNode = (ibex::ExprNode*)&product(*bwdDerivatives[node][outVar],
                                                             *getDerivativeWRTChildNode(node, 0))
                                           .simplify(0);

                if (node->opRoundType == Node::FL32 && operand->opRoundType == Node::FL64) {
                    typeCastRndVal = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(
                            node->roundingAmount[node->opRoundType]);
                } else {
                    typeCastRndVal = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(0);
                }


                if (contains(bwdDerivatives[operand], outVar)) {
                    bwdDerivatives[operand][outVar] = (ibex::ExprNode*)&(*bwdDerivatives[operand][outVar]
                                                                         + *derivThroughNode);
                } else {
                    bwdDerivatives[operand][outVar] = (ibex::ExprNode*)&(*derivThroughNode);
                }

                if (contains(typeCastRnd[operand], outVar)) {
                    typeCastRnd[operand][outVar] = (ibex::ExprNode*)&(*typeCastRnd[operand][outVar] + *typeCastRndVal);
                } else {
                    typeCastRnd[operand][outVar] = (ibex::ExprNode*)&(*typeCastRndVal);
                }

                if (logging && logging->level <= LogLevel::DEBUG) {
                    std::cout << *node->getExprNode() << " wrt " << *operand->getExprNode() << " : "
                              << *derivThroughNode << std::endl;
                    std::cout << "Derivative so far of " << *outVar->getExprNode() << " wrt " << *operand->getExprNode()
                              << " : " << *bwdDerivatives[operand][outVar] << std::endl;
                }

                // Add child to nextWorkList
                nextWorkList.insert(operand);
                // Increment number of parents of child that have been processed
                numParentsOfNode[operand]++;
                break;
            case BINARY_OP:
                // Computing the backward derivative of outVar with respect to node's children
                leftOperand = ((BinaryOp*)node)->leftOperand;
                derivLeftThroughNode = (ibex::ExprNode*)&product(*bwdDerivatives[node][outVar],
                                                                 *getDerivativeWRTChildNode(node, 0))
                                               .simplify(0);
                if (node->opRoundType == Node::FL32 && leftOperand->opRoundType == Node::FL64) {
                    typeCastRndVal = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(
                            node->roundingAmount[node->opRoundType]);
                } else {
                    typeCastRndVal = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(0);
                }

                if (contains(bwdDerivatives[leftOperand], outVar)) {
                    bwdDerivatives[leftOperand][outVar] = (ibex::ExprNode*)&(*bwdDerivatives[leftOperand][outVar]
                                                                             + *derivLeftThroughNode);
                } else {
                    bwdDerivatives[leftOperand][outVar] = (ibex::ExprNode*)&(*derivLeftThroughNode);
                }

                if (contains(typeCastRnd[leftOperand], outVar)) {
                    typeCastRnd[leftOperand][outVar] = (ibex::ExprNode*)&(*typeCastRnd[leftOperand][outVar]
                                                                          + *typeCastRndVal);
                } else {
                    typeCastRnd[leftOperand][outVar] = (ibex::ExprNode*)&(*typeCastRndVal);
                }

                if (logging && logging->level <= LogLevel::DEBUG) {
                    std::cout << *node->getExprNode() << " wrt " << *leftOperand->getExprNode() << " : "
                              << *derivLeftThroughNode << std::endl;
                    std::cout << "Derivative so far of " << *outVar->getExprNode() << " wrt "
                              << *leftOperand->getExprNode() << " : " << *bwdDerivatives[leftOperand][outVar]
                              << std::endl;
                }

                rightOperand = ((BinaryOp*)node)->rightOperand;
                derivRightThroughNode = (ibex::ExprNode*)&product(*bwdDerivatives[node][outVar],
                                                                  *getDerivativeWRTChildNode(node, 1))
                                                .simplify(0);
                if (node->opRoundType == Node::FL32 && rightOperand->opRoundType == Node::FL64) {
                    typeCastRndVal = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(
                            node->roundingAmount[node->opRoundType]);
                } else {
                    typeCastRndVal = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(0);
                }


                if (contains(bwdDerivatives[rightOperand], outVar)) {
                    bwdDerivatives[rightOperand][outVar] = (ibex::ExprNode*)&(*bwdDerivatives[rightOperand][outVar]
                                                                              + *derivRightThroughNode);
                } else {
                    bwdDerivatives[rightOperand][outVar] = (ibex::ExprNode*)&(*derivRightThroughNode);
                }

                if (contains(typeCastRnd[rightOperand], outVar)) {
                    typeCastRnd[rightOperand][outVar] = (ibex::ExprNode*)&(*typeCastRnd[rightOperand][outVar]
                                                                           + *typeCastRndVal);
                } else {
                    typeCastRnd[rightOperand][outVar] = (ibex::ExprNode*)&(*typeCastRndVal);
                }

                if (logging && logging->level <= LogLevel::DEBUG) {
                    std::cout << *node->getExprNode() << " wrt " << *rightOperand->getExprNode() << " : "
                              << *derivRightThroughNode << std::endl;
                    std::cout << "Derivative so far of " << *outVar->getExprNode() << " wrt "
                              << *rightOperand->getExprNode() << " : " << *bwdDerivatives[rightOperand][outVar]
                              << std::endl;
                }

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

void ErrorAnalyzer::printBwdDerivative(Node* outNode, Node* WRTNode) {
    std::cout << *outNode->getExprNode() << " wrt " << *WRTNode->getExprNode() << " : "
              << *this->bwdDerivatives[WRTNode][outNode] << std::endl;
}

void ErrorAnalyzer::printBwdDerivativesIbexExprs() {
    std::cout << "Backward Derivatives: " << std::endl;
    for (auto& wrtNode : this->bwdDerivatives) {
        for (auto& outputNode : wrtNode.second) { printBwdDerivative(outputNode.first, wrtNode.first); }
    }
}

void ErrorAnalyzer::logBwdDerivative(Node* outNode, Node* WRTNode) {
    if (logging) {
        logging->debug("Backward derivative log for nodes ", outNode->id, " wrt ", WRTNode->id);
    }
}

void ErrorAnalyzer::logBwdDerivativesIbexExprs() {
    if (logging) {
        logging->debug("Backward Derivatives:");
        for (auto& wrtNode : this->bwdDerivatives) {
            for (auto& outputNode : wrtNode.second) { logBwdDerivative(outputNode.first, wrtNode.first); }
        }
    }
}

void ErrorAnalyzer::errorComputingDriver(const std::set<Node*>& candidate_nodes, IBEXInterface* ibexInterface) {
    if (logging) { logging->debug("Computing Error..."); }

    for (auto& output : candidate_nodes) {
        if (errorComputedNodes[output->depth].find(output) == errorComputedNodes[output->depth].end()) {
            errorComputing(output, ibexInterface);
        }

        errAccumulator[output] = (ibex::ExprNode*)&(
                *errAccumulator[output]
                // The power term is the value of double ULP. We dont multiply by it here so
                // optimizer can function better AND we get the optimal value in terms of number of
                // ULPs. If uncommenting, comment the power term in the error computation
                //            * pow(2, -53)
        );
    }

    if (logging) { logging->debug("Error Expressions generated!"); }
}

void ErrorAnalyzer::errorComputing(Node* node, IBEXInterface* ibexInterface) {
    Node *operand, *leftOperand, *middleOperand, *rightOperand;

    switch (node->type) {
        case DEFAULT:
        case INTEGER:
        case FLOAT:
        case DOUBLE:
        case FREE_VARIABLE:
        case VARIABLE:
            break;
        case UNARY_OP:
            operand = ((UnaryOp*)node)->operand;
            if (errorComputedNodes[operand->depth].find(operand) == errorComputedNodes[operand->depth].end()) {
                errorComputing(operand, ibexInterface);
            }
            break;
        case BINARY_OP:
            leftOperand = ((BinaryOp*)node)->leftOperand;
            if (errorComputedNodes[leftOperand->depth].find(leftOperand)
                == errorComputedNodes[leftOperand->depth].end()) {
                errorComputing(leftOperand, ibexInterface);
            }

            rightOperand = ((BinaryOp*)node)->rightOperand;
            if (errorComputedNodes[rightOperand->depth].find(rightOperand)
                == errorComputedNodes[rightOperand->depth].end()) {
                errorComputing(rightOperand, ibexInterface);
            }
            break;
        case TERNARY_OP:
            leftOperand = ((TernaryOp*)node)->leftOperand;
            if (errorComputedNodes[leftOperand->depth].find(leftOperand)
                == errorComputedNodes[leftOperand->depth].end()) {
                errorComputing(leftOperand, ibexInterface);
            }

            middleOperand = ((TernaryOp*)node)->middleOperand;
            if (errorComputedNodes[middleOperand->depth].find(middleOperand)
                == errorComputedNodes[middleOperand->depth].end()) {
                errorComputing(middleOperand, ibexInterface);
            }

            rightOperand = ((TernaryOp*)node)->rightOperand;
            if (errorComputedNodes[rightOperand->depth].find(rightOperand)
                == errorComputedNodes[rightOperand->depth].end()) {
                errorComputing(rightOperand, ibexInterface);
            }
            break;
    }

    if (errorComputedNodes[node->depth].find(node) == errorComputedNodes[node->depth].end()) {
        propagateError(node, ibexInterface);
    }
    errorComputedNodes[node->depth].insert(node);
}

void ErrorAnalyzer::propagateError(Node* node, IBEXInterface* ibexInterface) {
    std::vector<Node*> outputList = keys(bwdDerivatives[node]);

    for (Node* outVar : outputList) {
        if (logging && logging->level <= LogLevel::DEBUG) {
            logging->debug("Propagating error for ", outVar->id, " through node ", node->id);
            printBwdDerivative(outVar, node);
            std::cout << "absolute error:" << node->getAbsoluteError() << std::endl;
            std::cout << "OpRounding:" << node->getRounding() << std::endl;
            std::cout << "Type Cast Rounding:" << *typeCastRnd[node][outVar] << std::endl;
        }

        // Generate the error expression by computing the product of the Backward derivative of
        // outVar wrt node and the local_error (product of the expression corresponding the node and
        // (the operator rounding + type cast rounding) Add the type cast rounding to the nodes
        // rounding amount rounding is the amount to round at ULP level whereas rounding error is
        // the absolute amount of error introduced
        auto total_rounding = (ibex::ExprNode*)&(node->getRounding() + *typeCastRnd[node][outVar]);
        auto local_plus_type_cast_error
                = (ibex::ExprNode*)&product(node->getAbsoluteError(), *total_rounding).simplify(0);
        auto expr = (ibex::ExprNode*)&product(*bwdDerivatives[node][outVar], *local_plus_type_cast_error).simplify(0);

        perInstructionErrors[outVar].emplace_back(node, expr);

        if (contains(errAccumulator, outVar)) {
            errAccumulator[outVar] = (ibex::ExprNode*)&(*errAccumulator[outVar] + *expr);
        } else {
            errAccumulator[outVar] = (ibex::ExprNode*)&(*expr);
        }

        if (errAccumulator[outVar]->size > errorExpressionOperatorThreshold) {
            OptResult max_err = ibexInterface->findAbsMax(*errAccumulator[outVar]);
            errAccumulator[outVar] = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar((-max_err.result).mag());
            nodeNumOptCallsMap[outVar]++;
            if (logging) {
                logging->debug("Error Accumulator size exceeded ", errorExpressionOperatorThreshold, ". Concretizing error.");
            }
        }

        if (logging && logging->level <= LogLevel::DEBUG) {
            std::cout << "Error Accumulator for " << *outVar->getExprNode() << " : " << *errAccumulator[outVar]
                      << std::endl;
            std::cout << std::endl;
        }
    }
}

ibex::ExprNode* getDerivativeWRTChildNode(Node* node, int index) {
    Node* child = node->getChildNode(index);

    switch (node->type) {
        case NodeType::INTEGER:
        case NodeType::FLOAT:
        case NodeType::DOUBLE:
        case NodeType::FREE_VARIABLE:
        case NodeType::VARIABLE:
            return (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(0.0);
        case NodeType::UNARY_OP:
            switch (((UnaryOp*)node)->op) {
                case Node::NEG:
                    return (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(-1);
                case Node::SIN:
                    return (ibex::ExprNode*)&cos(*child->getExprNode());
                case Node::COS:
                    return (ibex::ExprNode*)&sin(-*child->getExprNode());
                case Node::TAN:
                    return (ibex::ExprNode*)&(1.0 / sqr(cos(*child->getExprNode())));
                case Node::SINH:
                    return (ibex::ExprNode*)&(exp(*child->getExprNode()) - exp(-*child->getExprNode()) / 2.0);
                case Node::COSH:
                    return (ibex::ExprNode*)&(exp(*child->getExprNode()) + exp(-*child->getExprNode()) / 2.0);
                case Node::TANH:
                    return (ibex::ExprNode*)&(sinh(*child->getExprNode()) / cosh(*child->getExprNode()));
                case Node::ASIN:
                    return (ibex::ExprNode*)&(1.0 / sqrt(1.0 - sqr(*child->getExprNode())));
                case Node::ACOS:
                    return (ibex::ExprNode*)&(-1.0 / sqrt(1.0 - sqr(*child->getExprNode())));
                case Node::ATAN:
                    return (ibex::ExprNode*)&(1.0 / (1.0 + sqr(*child->getExprNode())));
                case Node::LOG:
                    return (ibex::ExprNode*)&(1.0 / (*child->getExprNode() * log(10.0)));
                case Node::SQRT:
                    return (ibex::ExprNode*)&(1.0 / (2.0 * sqrt(*child->getExprNode())));
                case Node::EXP:
                    return child->getExprNode();
                case Node::FPTRUNC:
                case Node::FPEXT:
                    return (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(1);
                default:
                    if (logging) {
                        logging->error("Should not be here. Unknown unary operator or not a unary operator.");
                    }
                    break;
            }
            break;
        case NodeType::BINARY_OP:
            switch (((BinaryOp*)node)->op) {
                case Node::ADD:
                    return (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(1);
                case Node::SUB:
                    if (index == 0) {
                        return (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(1);
                    } else {
                        return (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(-1);
                    }
                case Node::MUL:
                    if (index == 0) {
                        return ((BinaryOp*)node)->rightOperand->getExprNode();
                    } else {
                        return ((BinaryOp*)node)->leftOperand->getExprNode();
                    }
                case Node::DIV:
                    if (index == 0) {
                        return (ibex::ExprNode*)&(1.0 / *((BinaryOp*)node)->rightOperand->getExprNode());
                    } else if (index == 1) {
                        return (ibex::ExprNode*)&(-*((BinaryOp*)node)->leftOperand->getExprNode()
                                                  / sqr(*((BinaryOp*)node)->rightOperand->getExprNode()));
                    }
                default:
                    if (logging) {
                        logging->error("Should not be here. Unknown binary operator or not a binary operator.");
                    }
                    break;
            }
            break;
        case NodeType::TERNARY_OP:
            break;
        default:
            if (logging) {
                logging->critical("Unknown node type");
            }
    }

    return nullptr;
}

template<class T1, class T2>
std::vector<T1> keys(std::map<T1, T2> map) {
    std::vector<T1> keys;
    for (auto& key_value : map) { keys.push_back(key_value.first); }
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
    } else {
        map[key] = defaultVal;
        return defaultVal;
    }
}

std::map<Node*, std::vector<InstructionErrorInfo>> ErrorAnalyzer::getInstructionErrorBreakdown(IBEXInterface* ibexInterface, Graph* graph) {
    std::map<Node*, std::vector<InstructionErrorInfo>> result;

    for (const auto& [outputNode, instructionErrorPairs] : perInstructionErrors) {
        std::vector<InstructionErrorInfo> instructionInfos;
        std::map<Node*, ibex::ExprNode*> errorMap;

        // Build a map of nodes to their error expressions for quick lookup
        for (const auto& [instrNode, errorExpr] : instructionErrorPairs) {
            errorMap[instrNode] = errorExpr;
        }

        // Collect ALL nodes with metadata (if graph is provided)
        std::vector<Node*> allInstructionNodes;
        if (graph != nullptr) {
            for (Node* node : graph->nodes) {
                if (node->hasMetadata()) {
                    allInstructionNodes.push_back(node);
                }
            }
        } else {
            // Fallback: only use nodes from perInstructionErrors
            for (const auto& [instrNode, errorExpr] : instructionErrorPairs) {
                allInstructionNodes.push_back(instrNode);
            }
        }

        int index = 0;
        double totalError = 0.0;

        // First pass: calculate total error from nodes with error contributions
        for (const auto& [instrNode, errorExpr] : instructionErrorPairs) {
            OptResult maxErr = ibexInterface->findAbsMax(*errorExpr);
            totalError += maxErr.result.mag();
        }

        // Second pass: populate instruction info for ALL instructions
        double cumulativeError = 0.0;
        for (Node* instrNode : allInstructionNodes) {
            InstructionErrorInfo info;
            info.nodeId = instrNode->id;
            info.instructionIndex = index++;

            // Extract information from node metadata
            if (instrNode->hasMetadata()) {
                InstructionMetadata* metadata = instrNode->getMetadata();
                info.instructionName = metadata->getDisplayName();
                info.instructionType = metadata->instructionOpcode;
                info.sourceLocation = metadata->sourceLocation;
                info.irRepresentation = metadata->irRepresentation;
            } else {
                // Fallback to node type-based naming
                info.instructionName = "node_" + std::to_string(instrNode->id);
                switch (instrNode->type) {
                case VARIABLE:
                    info.instructionType = "VARIABLE";
                    break;
                case FREE_VARIABLE:
                    info.instructionType = "FREE_VARIABLE";
                    break;
                case UNARY_OP: {
                    auto* unaryNode = static_cast<UnaryOp*>(instrNode);
                    info.instructionType = getOpString(unaryNode->op);
                    break;
                }
                case BINARY_OP: {
                    auto* binaryNode = static_cast<BinaryOp*>(instrNode);
                    info.instructionType = getOpString(binaryNode->op);
                    break;
                }
                case TERNARY_OP: {
                    auto* ternaryNode = static_cast<TernaryOp*>(instrNode);
                    info.instructionType = getOpString(ternaryNode->op);
                    break;
                }
                case INTEGER:
                    info.instructionType = "INTEGER";
                    break;
                case FLOAT:
                    info.instructionType = "FLOAT";
                    break;
                case DOUBLE:
                    info.instructionType = "DOUBLE";
                    break;
                default:
                    info.instructionType = "UNKNOWN";
                    break;
                }
            }

            // Calculate error metrics
            auto errorIt = errorMap.find(instrNode);
            if (errorIt != errorMap.end()) {
                // This node has error contribution
                OptResult maxErr = ibexInterface->findAbsMax(*errorIt->second);
                info.errorContribution = maxErr.result.mag();
                info.errorBounds = maxErr.result;
                cumulativeError += info.errorContribution;
            } else {
                // This node has no error contribution
                info.errorContribution = 0.0;
                info.errorBounds = ibex::Interval(0.0, 0.0);
            }

            info.cumulativeError = cumulativeError;

            // Calculate percentage contribution
            if (totalError > 0.0) {
                info.percentageContribution = (info.errorContribution / totalError) * 100.0;
            }

            instructionInfos.push_back(info);
        }

        // Sort by error contribution (descending)
        std::sort(instructionInfos.begin(), instructionInfos.end());

        result[outputNode] = instructionInfos;
    }

    return result;
}
