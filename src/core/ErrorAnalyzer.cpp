#include "cire/core/ErrorAnalyzer.h"
#include "cire/core/Graph.hpp"
#include "cire/core/Results.h"
#include "cire/interfaces/Logging.h"

#include <algorithm>

std::string getOpString(ir::Node::OpType op) {
    switch (op) {
        case ir::Node::OpType::ADD:
            return "ADD";
        case ir::Node::OpType::SUB:
            return "SUB";
        case ir::Node::OpType::MUL:
            return "MUL";
        case ir::Node::OpType::DIV:
            return "DIV";
        case ir::Node::OpType::NEG:
            return "NEG";
        case ir::Node::OpType::SIN:
            return "SIN";
        case ir::Node::OpType::COS:
            return "COS";
        case ir::Node::OpType::TAN:
            return "TAN";
        case ir::Node::OpType::SINH:
            return "SINH";
        case ir::Node::OpType::COSH:
            return "COSH";
        case ir::Node::OpType::TANH:
            return "TANH";
        case ir::Node::OpType::ASIN:
            return "ASIN";
        case ir::Node::OpType::ACOS:
            return "ACOS";
        case ir::Node::OpType::ATAN:
            return "ATAN";
        case ir::Node::OpType::LOG:
            return "LOG";
        case ir::Node::OpType::SQRT:
            return "SQRT";
        case ir::Node::OpType::EXP:
            return "EXP";
        case ir::Node::OpType::FMA:
            return "FMA";
        case ir::Node::OpType::FPTRUNC:
            return "FPTRUNC";
        case ir::Node::OpType::FPEXT:
            return "FPEXT";
        default:
            return "UNKNOWN";
    }
}

ErrorAnalyzer::ErrorAnalyzer() = default;

bool ErrorAnalyzer::parentsVisited(ir::Node* node) { return numParentsOfNode[node] >= parentsOfNode[node].size(); }

void ErrorAnalyzer::derivativeComputingDriver() {
    if (logging) logging->debug("Computing Derivatives...");

    int next_depth = -1;

    // Iterate all nodes in the worklist
    while (!workList.empty()) {
        ir::Node* node = *workList.begin();
        workList.erase(node);

        int current_depth = int(node->depth);
        next_depth = current_depth - 1;
        // If node contains a constant, add it to completed list as you cannot
        // compute its derivative.
        if (derivativeComputedNodes[current_depth].find(node) != derivativeComputedNodes[current_depth].end()) {
            // If derivative of node has already been computed, move on.
        }
        // These are constants and their derivatives are 0
        else if (node->type == ir::Node::Type::INTEGER || node->type == ir::Node::Type::FLOAT
                 || node->type == ir::Node::Type::DOUBLE) {
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
                         [&next_depth](ir::Node* node) { return node->depth == next_depth; });
        }
    }

    if (logging && logging->level <= LogLevel::DEBUG) {
        printBwdDerivativesIbexExprs();
        std::cout << '\n';
    }

    if (logging) logging->debug("Derivatives computed!");
}

// Compute the Backward derivative of outVar with respect to node's children by using the chain rule
void ErrorAnalyzer::derivativeComputing(ir::Node* node) {
    // TODO: Type rounding handled only for FL64 --> FL32. Handle for other cases.
    std::vector<ir::Node*> outputList = keys(bwdDerivatives[node]);
    for (ir::Node* outVar : outputList) {
        assert(bwdDerivatives[node][outVar] != nullptr && "Derivative of output wrt node should have been computed\n");

        ir::Node* operand;
        ir::Node* leftOperand;
        ir::Node* rightOperand;
        ibex::ExprNode* derivThroughNode;
        ibex::ExprNode* derivLeftThroughNode;
        ibex::ExprNode* derivRightThroughNode;
        ibex::ExprNode* typeCastRndVal;
        switch (node->type) {
            case ir::Node::Type::DEFAULT:
            case ir::Node::Type::INTEGER:
            case ir::Node::Type::FLOAT:
            case ir::Node::Type::DOUBLE:
            case ir::Node::Type::FREE_VARIABLE:
            case ir::Node::Type::VARIABLE:
                break;
            case ir::Node::Type::UNARY_OP:
                operand = (dynamic_cast<ir::UnaryOp*>(node))->operand;
                derivThroughNode = (ibex::ExprNode*)&product(*bwdDerivatives[node][outVar],
                                                             *getDerivativeWRTChildNode(node, 0))
                                           .simplify(0);

                if (node->opRoundType == ir::Node::RoundingType::FL32
                    && operand->opRoundType == ir::Node::RoundingType::FL64) {
                    typeCastRndVal = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(
                            node->roundingAmount[node->opRoundType]);
                } else {
                    typeCastRndVal = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(0);
                }


                if (contains(bwdDerivatives[operand], outVar)) {
                    bwdDerivatives[operand][outVar] = (ibex::ExprNode*)&(*bwdDerivatives[operand][outVar]
                                                                         + *derivThroughNode);
                } else {
                    bwdDerivatives[operand][outVar] = &*derivThroughNode;
                }

                if (contains(typeCastRnd[operand], outVar)) {
                    typeCastRnd[operand][outVar] = (ibex::ExprNode*)&(*typeCastRnd[operand][outVar] + *typeCastRndVal);
                } else {
                    typeCastRnd[operand][outVar] = &*typeCastRndVal;
                }

                if (logging && logging->level <= LogLevel::DEBUG) {
                    std::cout << *node->getExprNode() << " wrt " << *operand->getExprNode() << " : "
                              << *derivThroughNode << '\n';
                    std::cout << "Derivative so far of " << *outVar->getExprNode() << " wrt " << *operand->getExprNode()
                              << " : " << *bwdDerivatives[operand][outVar] << '\n';
                }

                // Add child to nextWorkList
                nextWorkList.insert(operand);
                // Increment number of parents of child that have been processed
                numParentsOfNode[operand]++;
                break;
            case ir::Node::Type::BINARY_OP:
                // Computing the backward derivative of outVar with respect to node's children
                leftOperand = dynamic_cast<ir::BinaryOp*>(node)->leftOperand;
                derivLeftThroughNode = (ibex::ExprNode*)&product(*bwdDerivatives[node][outVar],
                                                                 *getDerivativeWRTChildNode(node, 0))
                                               .simplify(0);
                if (node->opRoundType == ir::Node::RoundingType::FL32
                    && leftOperand->opRoundType == ir::Node::RoundingType::FL64) {
                    typeCastRndVal = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(
                            node->roundingAmount[node->opRoundType]);
                } else {
                    typeCastRndVal = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(0);
                }

                if (contains(bwdDerivatives[leftOperand], outVar)) {
                    bwdDerivatives[leftOperand][outVar] = (ibex::ExprNode*)&(*bwdDerivatives[leftOperand][outVar]
                                                                             + *derivLeftThroughNode);
                } else {
                    bwdDerivatives[leftOperand][outVar] = &*derivLeftThroughNode;
                }

                if (contains(typeCastRnd[leftOperand], outVar)) {
                    typeCastRnd[leftOperand][outVar] = (ibex::ExprNode*)&(*typeCastRnd[leftOperand][outVar]
                                                                          + *typeCastRndVal);
                } else {
                    typeCastRnd[leftOperand][outVar] = &*typeCastRndVal;
                }

                if (logging && logging->level <= LogLevel::DEBUG) {
                    std::cout << *node->getExprNode() << " wrt " << *leftOperand->getExprNode() << " : "
                              << *derivLeftThroughNode << '\n';
                    std::cout << "Derivative so far of " << *outVar->getExprNode() << " wrt "
                              << *leftOperand->getExprNode() << " : " << *bwdDerivatives[leftOperand][outVar] << '\n';
                }

                rightOperand = dynamic_cast<ir::BinaryOp*>(node)->rightOperand;
                derivRightThroughNode = (ibex::ExprNode*)&product(*bwdDerivatives[node][outVar],
                                                                  *getDerivativeWRTChildNode(node, 1))
                                                .simplify(0);
                if (node->opRoundType == ir::Node::RoundingType::FL32
                    && rightOperand->opRoundType == ir::Node::RoundingType::FL64) {
                    typeCastRndVal = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(
                            node->roundingAmount[node->opRoundType]);
                } else {
                    typeCastRndVal = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(0);
                }


                if (contains(bwdDerivatives[rightOperand], outVar)) {
                    bwdDerivatives[rightOperand][outVar] = (ibex::ExprNode*)&(*bwdDerivatives[rightOperand][outVar]
                                                                              + *derivRightThroughNode);
                } else {
                    bwdDerivatives[rightOperand][outVar] = &*derivRightThroughNode;
                }

                if (contains(typeCastRnd[rightOperand], outVar)) {
                    typeCastRnd[rightOperand][outVar] = (ibex::ExprNode*)&(*typeCastRnd[rightOperand][outVar]
                                                                           + *typeCastRndVal);
                } else {
                    typeCastRnd[rightOperand][outVar] = &*typeCastRndVal;
                }

                if (logging && logging->level <= LogLevel::DEBUG) {
                    std::cout << *node->getExprNode() << " wrt " << *rightOperand->getExprNode() << " : "
                              << *derivRightThroughNode << '\n';
                    std::cout << "Derivative so far of " << *outVar->getExprNode() << " wrt "
                              << *rightOperand->getExprNode() << " : " << *bwdDerivatives[rightOperand][outVar] << '\n';
                }

                // Add children to nextWorkList
                nextWorkList.insert(leftOperand);
                nextWorkList.insert(rightOperand);

                // Increment number of parents of children that have been processed
                numParentsOfNode[leftOperand]++;
                numParentsOfNode[rightOperand]++;
                break;
            case ir::Node::Type::TERNARY_OP:
                // TODO: Complete this on adding ternary operations
                break;
        }
    }

    derivativeComputedNodes[int(node->depth)].insert(node);
}

void ErrorAnalyzer::printBwdDerivative(ir::Node* outNode, ir::Node* WRTNode) {
    std::cout << *outNode->getExprNode() << " wrt " << *WRTNode->getExprNode() << " : "
              << *this->bwdDerivatives[WRTNode][outNode] << '\n';
}

void ErrorAnalyzer::printBwdDerivativesIbexExprs() {
    std::cout << "Backward Derivatives: \n";
    for (auto& wrtNode : this->bwdDerivatives) {
        for (auto& outputNode : wrtNode.second) printBwdDerivative(outputNode.first, wrtNode.first);
    }
}

void ErrorAnalyzer::logBwdDerivative(ir::Node* outNode, ir::Node* WRTNode) {
    if (logging) { logging->debug("Backward derivative log for nodes ", outNode->id, " wrt ", WRTNode->id); }
}

void ErrorAnalyzer::logBwdDerivativesIbexExprs() {
    if (logging) {
        logging->debug("Backward Derivatives:");
        for (auto& wrtNode : this->bwdDerivatives) {
            for (auto& outputNode : wrtNode.second) logBwdDerivative(outputNode.first, wrtNode.first);
        }
    }
}

void ErrorAnalyzer::errorComputingDriver(const std::set<ir::Node*>& candidate_nodes, IBEXInterface* ibexInterface) {
    if (logging) logging->debug("Computing Error...");

    for (const auto& output : candidate_nodes) {
        if (errorComputedNodes[int(output->depth)].find(output) == errorComputedNodes[int(output->depth)].end()) {
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

    if (logging) logging->debug("Error Expressions generated!");
}

void ErrorAnalyzer::errorComputing(ir::Node* node, IBEXInterface* ibexInterface) {
    ir::Node* operand;
    ir::Node* leftOperand;
    ir::Node* middleOperand;
    ir::Node* rightOperand;

    switch (node->type) {
        case ir::Node::Type::DEFAULT:
        case ir::Node::Type::INTEGER:
        case ir::Node::Type::FLOAT:
        case ir::Node::Type::DOUBLE:
        case ir::Node::Type::FREE_VARIABLE:
        case ir::Node::Type::VARIABLE:
            break;
        case ir::Node::Type::UNARY_OP:
            operand = (dynamic_cast<ir::UnaryOp*>(node))->operand;
            if (errorComputedNodes[int(operand->depth)].find(operand)
                == errorComputedNodes[int(operand->depth)].end()) {
                errorComputing(operand, ibexInterface);
            }
            break;
        case ir::Node::Type::BINARY_OP:
            leftOperand = dynamic_cast<ir::BinaryOp*>(node)->leftOperand;
            if (errorComputedNodes[int(leftOperand->depth)].find(leftOperand)
                == errorComputedNodes[int(leftOperand->depth)].end()) {
                errorComputing(leftOperand, ibexInterface);
            }

            rightOperand = dynamic_cast<ir::BinaryOp*>(node)->rightOperand;
            if (errorComputedNodes[int(rightOperand->depth)].find(rightOperand)
                == errorComputedNodes[int(rightOperand->depth)].end()) {
                errorComputing(rightOperand, ibexInterface);
            }
            break;
        case ir::Node::Type::TERNARY_OP:
            leftOperand = dynamic_cast<ir::TernaryOp*>(node)->leftOperand;
            if (errorComputedNodes[int(leftOperand->depth)].find(leftOperand)
                == errorComputedNodes[int(leftOperand->depth)].end()) {
                errorComputing(leftOperand, ibexInterface);
            }

            middleOperand = dynamic_cast<ir::TernaryOp*>(node)->middleOperand;
            if (errorComputedNodes[int(middleOperand->depth)].find(middleOperand)
                == errorComputedNodes[int(middleOperand->depth)].end()) {
                errorComputing(middleOperand, ibexInterface);
            }

            rightOperand = dynamic_cast<ir::TernaryOp*>(node)->rightOperand;
            if (errorComputedNodes[int(rightOperand->depth)].find(rightOperand)
                == errorComputedNodes[int(rightOperand->depth)].end()) {
                errorComputing(rightOperand, ibexInterface);
            }
            break;
    }

    if (errorComputedNodes[int(node->depth)].find(node) == errorComputedNodes[int(node->depth)].end()) {
        propagateError(node, ibexInterface);
    }
    errorComputedNodes[int(node->depth)].insert(node);
}

void ErrorAnalyzer::propagateError(ir::Node* node, IBEXInterface* ibexInterface) {
    std::vector<ir::Node*> outputList = keys(bwdDerivatives[node]);

    for (ir::Node* outVar : outputList) {
        if (logging && logging->level <= LogLevel::DEBUG) {
            logging->debug("Propagating error for ", outVar->id, " through node ", node->id);
            printBwdDerivative(outVar, node);
            std::cout << "absolute error:" << node->getAbsoluteError() << '\n';
            std::cout << "OpRounding:" << node->getRounding() << '\n';
            std::cout << "Type Cast Rounding:" << *typeCastRnd[node][outVar] << '\n';
        }

        // Store the backwards derivative directly instead of error expression
        // The concrete derivative will be evaluated at worst-case input point later
        auto* bwd_derivative = bwdDerivatives[node][outVar];
        perInstructionErrors[outVar].emplace_back(node, bwd_derivative);

        // Still compute full error expression for error accumulator (used for global optimization)
        auto* total_rounding = (ibex::ExprNode*)&(node->getRounding() + *typeCastRnd[node][outVar]);
        auto* local_plus_type_cast_error
                = (ibex::ExprNode*)&product(node->getAbsoluteError(), *total_rounding).simplify(0);
        auto* expr = (ibex::ExprNode*)&product(*bwd_derivative, *local_plus_type_cast_error).simplify(0);

        if (contains(errAccumulator, outVar)) {
            errAccumulator[outVar] = (ibex::ExprNode*)&(*errAccumulator[outVar] + *expr);
        } else {
            errAccumulator[outVar] = &*expr;
        }

        if (errAccumulator[outVar]->size > errorExpressionOperatorThreshold) {
            OptResult max_err = ibexInterface->findAbsMax(*errAccumulator[outVar]);
            errAccumulator[outVar] = (ibex::ExprNode*)&ibex::ExprConstant::new_scalar((-max_err.result).mag());
            nodeNumOptCallsMap[outVar]++;
            if (logging) {
                logging->debug("Error Accumulator size exceeded ", errorExpressionOperatorThreshold,
                               ". Concretizing error.");
            }
        }

        if (logging && logging->level <= LogLevel::DEBUG) {
            std::cout << "Error Accumulator for " << *outVar->getExprNode() << " : " << *errAccumulator[outVar]
                      << "\n\n";
        }
    }
}

ibex::ExprNode* getDerivativeWRTChildNode(ir::Node* node, int index) {
    ir::Node* child = node->getChildNode(index);

    switch (node->type) {
        case ir::Node::Type::INTEGER:
        case ir::Node::Type::FLOAT:
        case ir::Node::Type::DOUBLE:
        case ir::Node::Type::FREE_VARIABLE:
        case ir::Node::Type::VARIABLE:
            return (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(0.0);
        case ir::Node::Type::UNARY_OP:
            switch (dynamic_cast<ir::UnaryOp*>(node)->op) {
                case ir::Node::OpType::NEG:
                    return (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(-1);
                case ir::Node::OpType::SIN:
                    return (ibex::ExprNode*)&cos(*child->getExprNode());
                case ir::Node::OpType::COS:
                    return (ibex::ExprNode*)&sin(-*child->getExprNode());
                case ir::Node::OpType::TAN:
                    return (ibex::ExprNode*)&(1.0 / sqr(cos(*child->getExprNode())));
                case ir::Node::OpType::SINH:
                    return (ibex::ExprNode*)&(exp(*child->getExprNode()) - exp(-*child->getExprNode()) / 2.0);
                case ir::Node::OpType::COSH:
                    return (ibex::ExprNode*)&(exp(*child->getExprNode()) + exp(-*child->getExprNode()) / 2.0);
                case ir::Node::OpType::TANH:
                    return (ibex::ExprNode*)&(sinh(*child->getExprNode()) / cosh(*child->getExprNode()));
                case ir::Node::OpType::ASIN:
                    return (ibex::ExprNode*)&(1.0 / sqrt(1.0 - sqr(*child->getExprNode())));
                case ir::Node::OpType::ACOS:
                    return (ibex::ExprNode*)&(-1.0 / sqrt(1.0 - sqr(*child->getExprNode())));
                case ir::Node::OpType::ATAN:
                    return (ibex::ExprNode*)&(1.0 / (1.0 + sqr(*child->getExprNode())));
                case ir::Node::OpType::LOG:
                    return (ibex::ExprNode*)&(1.0 / (*child->getExprNode() * log(10.0)));
                case ir::Node::OpType::SQRT:
                    return (ibex::ExprNode*)&(1.0 / (2.0 * sqrt(*child->getExprNode())));
                case ir::Node::OpType::EXP:
                    return child->getExprNode();
                case ir::Node::OpType::FPTRUNC:
                case ir::Node::OpType::FPEXT:
                    return (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(1);
                default:
                    if (logging) {
                        logging->error("Should not be here. Unknown unary operator or not a unary operator.");
                    }
                    break;
            }
            break;
        case ir::Node::Type::BINARY_OP:
            switch (dynamic_cast<ir::BinaryOp*>(node)->op) {
                case ir::Node::OpType::ADD:
                    return (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(1);
                case ir::Node::OpType::SUB:
                    if (index == 0) {
                        return (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(1);
                    } else {
                        return (ibex::ExprNode*)&ibex::ExprConstant::new_scalar(-1);
                    }
                case ir::Node::OpType::MUL:
                    if (index == 0) {
                        return (dynamic_cast<ir::BinaryOp*>(node))->rightOperand->getExprNode();
                    } else {
                        return (dynamic_cast<ir::BinaryOp*>(node))->leftOperand->getExprNode();
                    }
                case ir::Node::OpType::DIV:
                    if (index == 0) {
                        return (ibex::ExprNode*)&(1.0
                                                  / *(dynamic_cast<ir::BinaryOp*>(node))->rightOperand->getExprNode());
                    } else if (index == 1) {
                        return (ibex::ExprNode*)&(
                                -*dynamic_cast<ir::BinaryOp*>(node)->leftOperand->getExprNode()
                                / sqr(*dynamic_cast<ir::BinaryOp*>(node)->rightOperand->getExprNode()));
                    }
                default:
                    if (logging) {
                        logging->error("Should not be here. Unknown binary operator or not a binary operator.");
                    }
                    break;
            }
            break;
        case ir::Node::Type::TERNARY_OP:
            break;
        default:
            if (logging) { logging->critical("Unknown node type"); }
    }

    return nullptr;
}

template<class T1, class T2>
std::vector<T1> keys(std::map<T1, T2> map) {
    std::vector<T1> keys;
    for (auto& key_value : map) keys.push_back(key_value.first);
    return keys;
}

template<class T1, class T2>
bool contains(std::map<T1, T2> map, T1 key) {
    return map.find(key) != map.end();
}

template<class T1, class T2>
T2 findWithDefaultInsertion(std::map<T1, T2> map, T1 key, T2 defaultVal) {
    auto it = map.find(key);
    if (it != map.end()) return it->second;
    map[key] = defaultVal;
    return defaultVal;
}

std::map<ir::Node*, std::vector<InstructionErrorInfo>>
ErrorAnalyzer::getInstructionErrorBreakdown(IBEXInterface* ibexInterface, Graph* graph) {
    std::map<ir::Node*, std::vector<InstructionErrorInfo>> result;

    for (const auto& [outputNode, instructionErrorPairs] : perInstructionErrors) {
        std::vector<InstructionErrorInfo> instructionInfos;
        std::map<ir::Node*, ibex::ExprNode*> errorMap;

        // Build a map of nodes to their error expressions for quick lookup
        for (const auto& [instrNode, errorExpr] : instructionErrorPairs) { errorMap[instrNode] = errorExpr; }

        // Collect ALL nodes with metadata (if graph is provided)
        std::vector<ir::Node*> allInstructionNodes;
        if (graph != nullptr) {
            for (ir::Node* node : graph->nodes) {
                if (node->hasMetadata()) { allInstructionNodes.push_back(node); }
            }
        } else {
            // Fallback: only use nodes from perInstructionErrors
            for (const auto& [instrNode, errorExpr] : instructionErrorPairs) {
                allInstructionNodes.push_back(instrNode);
            }
        }

        int index = 0;
        double totalError = 0.0;

        // Find the global worst-case input for this output by optimizing the total error accumulator
        OptResult globalWorstCase;
        if (contains(errAccumulator, outputNode)) {
            globalWorstCase = ibexInterface->findAbsMax(*errAccumulator[outputNode]);
        } else {
            // No error accumulator for this output, use zero input
            globalWorstCase.optimumPoint = ibex::IntervalVector(ibexInterface->getInputIntervals().size(), ibex::Interval::ZERO);
            globalWorstCase.result = ibex::Interval::ZERO;
        }

        // First compute total derivative magnitude for percentage calculations
        double totalDerivativeMagnitude = 0.0;
        for (ir::Node* instrNode : allInstructionNodes) {
            auto derivativeIt = errorMap.find(instrNode);
            if (derivativeIt != errorMap.end()) {
                try {
                    // Evaluate backwards derivative at the global worst-case input point
                    ibex::Interval derivative = ibexInterface->evalAtPoint(*derivativeIt->second, globalWorstCase.optimumPoint);
                    totalDerivativeMagnitude += abs(derivative.mid());
                } catch (...) {
                    // If evaluation fails, use heuristic based on expression size
                    double derivativeMagnitude = std::min(5.0, double(derivativeIt->second->size) / 5.0);
                    totalDerivativeMagnitude += derivativeMagnitude;
                }
            }
        }

        // Second pass: populate instruction info for ALL instructions
        double cumulativeError = 0.0;
        for (ir::Node* instrNode : allInstructionNodes) {
            InstructionErrorInfo info;
            info.nodeId = int(instrNode->id);
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
                    case ir::Node::Type::VARIABLE:
                        info.instructionType = "VARIABLE";
                        break;
                    case ir::Node::Type::FREE_VARIABLE:
                        info.instructionType = "FREE_VARIABLE";
                        break;
                    case ir::Node::Type::UNARY_OP: {
                        auto* unaryNode = dynamic_cast<ir::UnaryOp*>(instrNode);
                        info.instructionType = getOpString(unaryNode->op);
                        break;
                    }
                    case ir::Node::Type::BINARY_OP: {
                        auto* binaryNode = dynamic_cast<ir::BinaryOp*>(instrNode);
                        info.instructionType = getOpString(binaryNode->op);
                        break;
                    }
                    case ir::Node::Type::TERNARY_OP: {
                        auto* ternaryNode = dynamic_cast<ir::TernaryOp*>(instrNode);
                        info.instructionType = getOpString(ternaryNode->op);
                        break;
                    }
                    case ir::Node::Type::INTEGER:
                        info.instructionType = "INTEGER";
                        break;
                    case ir::Node::Type::FLOAT:
                        info.instructionType = "FLOAT";
                        break;
                    case ir::Node::Type::DOUBLE:
                        info.instructionType = "DOUBLE";
                        break;
                    default:
                        info.instructionType = "UNKNOWN";
                        break;
                }
            }

            // Calculate backwards derivative at worst-case input
            auto derivativeIt = errorMap.find(instrNode);
            if (derivativeIt != errorMap.end()) {
                try {
                    // Evaluate backwards derivative at the global worst-case input point
                    ibex::Interval derivative = ibexInterface->evalAtPoint(*derivativeIt->second, globalWorstCase.optimumPoint);
                    info.errorContribution = abs(derivative.mid()); // Use absolute value of derivative
                    info.errorBounds = derivative;
                    cumulativeError += info.errorContribution;
                    
                    // Debug output to understand the derivative values
                    if (logging && logging->level <= LogLevel::DEBUG) {
                        logging->debug("Node ", instrNode->id, " (", info.instructionName, 
                                     "): derivative = ", derivative, 
                                     ", magnitude = ", info.errorContribution,
                                     ", worst-case input = ", globalWorstCase.optimumPoint);
                    }
                } catch (...) {
                    // If evaluation fails, use heuristic based on expression size
                    double derivativeMagnitude = std::min(5.0, double(derivativeIt->second->size) / 5.0);
                    info.errorContribution = derivativeMagnitude;
                    info.errorBounds = ibex::Interval(-derivativeMagnitude, derivativeMagnitude);
                    cumulativeError += info.errorContribution;
                }
            } else {
                // This node has no backwards derivative
                info.errorContribution = 0.0;
                info.errorBounds = ibex::Interval(0.0, 0.0);
            }

            info.cumulativeError = cumulativeError;

            // Calculate percentage contribution based on total derivative magnitude
            if (totalDerivativeMagnitude > 0.0) { 
                info.percentageContribution = (info.errorContribution / totalDerivativeMagnitude) * 100.0; 
            }

            instructionInfos.push_back(info);
        }

        // Sort by error contribution (descending)
        std::sort(instructionInfos.begin(), instructionInfos.end());

        result[outputNode] = instructionInfos;
    }

    return result;
}
