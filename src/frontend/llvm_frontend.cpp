/**
 * @file util/llvm_frontend.cpp
 * @brief Implementation of the LLVM frontend for converting LLVM IR to a CIRE graph.
 *
 * This file implements functions to parse LLVM IR instructions and convert them into nodes in a CIRE graph,
 * handling various LLVM instructions and types with appropriate conversions and mappings.
 *
 * See the header file for documentation.
 */

#include "cire/frontend/llvm_frontend.h"

#include <llvm/IR/Constants.h>

void addDataForCreatedNode(llvm::Instruction& instr, Graph& graph, ir::Node* node) {
    if (instr.getType()->isHalfTy()) {
        node->setRoundingType(ir::Node::RoundingType::FL16);
        node->setRoundingFromType(ir::Node::RoundingType::FL16);
    } else if (instr.getType()->isFloatTy()) {
        node->setRoundingType(ir::Node::RoundingType::FL32);
        node->setRoundingFromType(ir::Node::RoundingType::FL32);
    } else if (instr.getType()->isDoubleTy()) {
        node->setRoundingType(ir::Node::RoundingType::FL64);
        node->setRoundingFromType(ir::Node::RoundingType::FL64);
    }

    // Create instruction metadata using SourceMapper
    auto metadata = graph.sourceMapper->createMetadata(&instr, instr.getFunction()->getName().str());

    // Ensure the instruction has a name for the metadata
    if (metadata->instructionName.empty()) {
        metadata->instructionName = std::string(instr.getOpcodeName()) + "_" + std::to_string(node->id);
    }

    // Attach metadata to the node
    node->setMetadata(std::move(metadata));

    // Register node in graph structures
    graph.nodes.insert(node);
    graph.depthTable[int(node->depth)].insert(node);
    graph.numOperatorsOutput++;

    // Register LLVM value to node mappings
    graph.registerLLVMNode(&instr, node);

    // Index by instruction type for fast queries
    graph.indexNodeByInstructionType(node, instr.getOpcodeName());

    // Add to symbol table if instruction has a name
    if (!instr.getName().empty()) { graph.symbolTables[graph.currentScope]->insert(instr.getName().str(), node); }
}

ir::Node* getNodeFromLLVMValue(llvm::Value* val, Graph& graph) {
    // Check if node already exists in the graph
    ir::Node* existingNode = graph.getNodeByLLVMValue(val);
    if (existingNode != nullptr) { return existingNode; }

    // Create new node for constants
    if (llvm::isa<llvm::ConstantData>(val)) {
        ir::Node* new_node = nullptr;

        if (val->getType()->isFloatingPointTy()) {
            auto* CD = llvm::dyn_cast<llvm::ConstantFP>(val);
            new_node = new ir::Double(CD->getValueAPF().convertToDouble());
        } else if (val->getType()->isIntegerTy()) {
            auto* CI = llvm::dyn_cast<llvm::ConstantInt>(val);
            new_node = new ir::Integer(int(CI->getSExtValue()));
        }

        if (new_node != nullptr) {
            // Create synthetic metadata for constant
            auto metadata = graph.sourceMapper->createMetadata(val);
            new_node->setMetadata(std::move(metadata));

            // Register in graph
            graph.nodes.insert(new_node);
            graph.registerLLVMNode(val, new_node);

            return new_node;
        }
    }

    return nullptr;
}

void parseInputsInLLVM(Graph& graph, llvm::Function& func,
                       const std::map<std::string, std::pair<double, double>>& inputBounds) {
    graph.createNewSymbolTable();

    // Iterate the function arguments
    for (auto& arg : func.args()) {
        llvm::Type* arg_type = arg.getType();
        ir::Node::RoundingType rounding_type;

        // Getting the CIRE type of argument corresponding the LLVM type
        switch (arg_type->getTypeID()) {
            case llvm::Type::TypeID::IntegerTyID: {
                rounding_type = ir::Node::RoundingType::INT;
                break;
            }
            case llvm::Type::TypeID::HalfTyID: {
                rounding_type = ir::Node::RoundingType::FL16;
                break;
            }
            case llvm::Type::TypeID::FloatTyID: {
                rounding_type = ir::Node::RoundingType::FL32;
                break;
            }
            case llvm::Type::TypeID::DoubleTyID: {
                rounding_type = ir::Node::RoundingType::FL64;
                break;
            }
            default: {
                llvm::outs() << "Unhandled Type:" << arg_type << "\n";
                exit(1);  // NOLINT
            }
        }

        auto* new_variable = new ir::VariableNode(rounding_type);

        // Create metadata for the variable node
        auto varMetadata = graph.sourceMapper->createMetadata(&arg, func.getName().str());
        new_variable->setMetadata(std::move(varMetadata));

        graph.nodes.insert(new_variable);
        graph.symbolTables[graph.currentScope]->insert(arg.getNameOrAsOperand(), new_variable);

        // Register the argument mapping
        graph.registerLLVMNode(&arg, new_variable);

        // Create corresponding free variable for input with custom or default bounds
        std::string paramName = arg.getNameOrAsOperand();
        auto boundsIter = inputBounds.find(paramName);

        ir::FreeVariable* free_var;
        if (boundsIter != inputBounds.end()) {
            // Use custom bounds from command-line argument
            double min = boundsIter->second.first;
            double max = boundsIter->second.second;
            free_var = new ir::FreeVariable(*new ibex::Interval(min, max), rounding_type);
        } else {
            // Use default bounds [-1.0, 1.0]
            free_var = new ir::FreeVariable(rounding_type);
        }

        auto freeVarMetadata = graph.sourceMapper->createSyntheticMetadata("input_" + paramName, "FREE_VARIABLE");
        free_var->setMetadata(std::move(freeVarMetadata));

        graph.inputs[paramName.c_str()] = free_var;
        graph.nodes.insert(free_var);
    }
}

void parseExprsInLLVM(Graph& graph, llvm::Function& func) {
    // TODO: Make this work for multiple Basic blocks after you have support for
    //  conditionals
    // assume we have just one basic block
    assert(func.size() == 1 && "Function has more than one basic block. Cant work.");

    // Get the basic block
    llvm::BasicBlock& BB = func.getEntryBlock();

    // Iterate over the instructions in the basic block
    for (auto& I : BB) {
        auto opcode = I.getOpcode();

        switch (opcode) {
            case llvm::Instruction::Ret: {
                if (I.getOperand(0)->getType()->isFloatingPointTy()) {
                    graph.outputs.push_back(I.getOperand(0)->getName().str());
                }
                break;
            }
            case llvm::Instruction::FNeg: {
                if (I.getType()->isFloatingPointTy()) {
                    auto* op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    addDataForCreatedNode(I, graph, &(-*op1));
                }
                break;
            }
            case llvm::Instruction::FAdd: {
                if (I.getType()->isFloatingPointTy()) {
                    auto* op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    auto* op2 = getNodeFromLLVMValue(I.getOperand(1), graph);

                    addDataForCreatedNode(I, graph, &(*op1 + op2));
                }
                break;
            }
            case llvm::Instruction::FSub: {
                if (I.getType()->isFloatingPointTy()) {
                    auto* op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    auto* op2 = getNodeFromLLVMValue(I.getOperand(1), graph);

                    addDataForCreatedNode(I, graph, &(*op1 - op2));
                }
                break;
            }
            case llvm::Instruction::FMul: {
                if (I.getType()->isFloatingPointTy()) {
                    auto* op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    auto* op2 = getNodeFromLLVMValue(I.getOperand(1), graph);

                    addDataForCreatedNode(I, graph, &(*op1 * op2));
                }
                break;
            }
            case llvm::Instruction::FDiv: {
                if (I.getType()->isFloatingPointTy()) {
                    auto* op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    auto* op2 = getNodeFromLLVMValue(I.getOperand(1), graph);

                    addDataForCreatedNode(I, graph, &(*op1 / op2));
                }
                break;
            }
            case llvm::Instruction::FPTrunc:
                if (I.getType()->isFloatingPointTy()) {
                    auto* op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    ir::Node::RoundingType rounding_type;
                    if (llvm::dyn_cast<llvm::FPTruncInst>(&I)->getDestTy()->isFloatTy()) {
                        rounding_type = ir::Node::RoundingType::FL32;
                    } else if (llvm::dyn_cast<llvm::FPTruncInst>(&I)->getDestTy()->isDoubleTy()) {
                        rounding_type = ir::Node::RoundingType::FL64;
                    } else {
                        llvm::outs() << "Incorrect Destination type:" << I.getType() << "\n";
                        exit(1);  // NOLINT
                    }

                    addDataForCreatedNode(I, graph, &fptrunc(*op1, rounding_type));
                }
                break;
            case llvm::Instruction::FPExt:
                if (I.getType()->isFloatingPointTy()) {
                    auto* op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    ir::Node::RoundingType rounding_type;
                    if (llvm::dyn_cast<llvm::FPExtInst>(&I)->getDestTy()->isFloatTy()) {
                        rounding_type = ir::Node::RoundingType::FL32;
                    } else if (llvm::dyn_cast<llvm::FPExtInst>(&I)->getDestTy()->isDoubleTy()) {
                        rounding_type = ir::Node::RoundingType::FL64;
                    } else {
                        llvm::outs() << "Incorrect Destination type:" << I.getType() << "\n";
                        exit(1);  // NOLINT
                    }

                    addDataForCreatedNode(I, graph, &fpext(*op1, rounding_type));
                }
                break;
            case llvm::Instruction::PtrToInt:
            case llvm::Instruction::IntToPtr:
            case llvm::Instruction::BitCast:
            case llvm::Instruction::AddrSpaceCast:
            case llvm::Instruction::ICmp:
            case llvm::Instruction::FCmp:
            case llvm::Instruction::Call: {
                auto* CI = llvm::dyn_cast<llvm::CallInst>(&I);
                if (CI->getCalledFunction()->arg_size() == 1) {
                    auto CalledFunctionName = CI->getCalledFunction()->getName().str();
                    auto* op1 = getNodeFromLLVMValue(CI->getOperand(0), graph);

                    if (CalledFunctionName == "sin" || CalledFunctionName == "sinf"
                        || CI->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::sin) {
                        addDataForCreatedNode(I, graph, &sin(*op1));
                    } else if (CalledFunctionName == "cos" || CalledFunctionName == "cosf"
                               || CI->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::cos) {
                        addDataForCreatedNode(I, graph, &cos(*op1));
                    } else if (CalledFunctionName == "tan" || CalledFunctionName == "tanf"
                               || CI->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::tan) {
                        addDataForCreatedNode(I, graph, &tan(*op1));
                    } else if (CalledFunctionName == "sinh" || CalledFunctionName == "sinhf"
                               || CI->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::sinh) {
                        addDataForCreatedNode(I, graph, &sinh(*op1));
                    } else if (CalledFunctionName == "cosh" || CalledFunctionName == "coshf"
                               || CI->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::cosh) {
                        addDataForCreatedNode(I, graph, &cosh(*op1));
                    } else if (CalledFunctionName == "tanh" || CalledFunctionName == "tanhf"
                               || CI->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::tanh) {
                        addDataForCreatedNode(I, graph, &tanh(*op1));
                    } else if (CalledFunctionName == "asin" || CalledFunctionName == "asinf"
                               || CI->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::asin) {
                        addDataForCreatedNode(I, graph, &asin(*op1));
                    } else if (CalledFunctionName == "acos" || CalledFunctionName == "acosf"
                               || CI->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::acos) {
                        addDataForCreatedNode(I, graph, &acos(*op1));
                    } else if (CalledFunctionName == "atan" || CalledFunctionName == "atanf"
                               || CI->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::atan) {
                        addDataForCreatedNode(I, graph, &atan(*op1));
                    } else if (CalledFunctionName == "log" || CalledFunctionName == "logf"
                               || CI->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::log) {
                        addDataForCreatedNode(I, graph, &log(*op1));
                    } else if (CalledFunctionName == "sqrt" || CalledFunctionName == "sqrtf"
                               || CI->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::sqrt) {
                        addDataForCreatedNode(I, graph, &sqrt(*op1));
                    } else if (CalledFunctionName == "exp" || CalledFunctionName == "expf"
                               || CI->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::exp) {
                        addDataForCreatedNode(I, graph, &exp(*op1));
                    } else {
                        llvm::outs() << "Unhandled Function in Call Instruction:" << I << "\n";
                        exit(1);  // NOLINT
                    }
                } else if (CI->getCalledFunction()->arg_size() == 3) {
                    // auto CalledFunctionName = CI->getCalledFunction()->getName().str();
                    auto* op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    auto* op2 = getNodeFromLLVMValue(I.getOperand(1), graph);
                    auto* op3 = getNodeFromLLVMValue(I.getOperand(2), graph);

                    if (CI->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::fma
                        || CI->getCalledFunction()->getIntrinsicID() == llvm::Intrinsic::fmuladd) {
                        addDataForCreatedNode(I, graph, &fma(*op1, *op2, *op3));
                    } else {
                        llvm::outs() << "Unhandled Function in Call Instruction:" << I << "\n";
                        exit(1);  // NOLINT
                    }
                } else {
                    llvm::outs() << "Function with " << CI->getCalledFunction()->arg_size()
                                 << " arguments in Call Instruction not handled:" << I << "\n";
                    exit(1);  // NOLINT
                }

                break;
            }
            case llvm::Instruction::Br:
            case llvm::Instruction::Switch:
            case llvm::Instruction::IndirectBr:
            case llvm::Instruction::Invoke:
            case llvm::Instruction::Resume:
            case llvm::Instruction::Unreachable:
            case llvm::Instruction::CleanupRet:
            case llvm::Instruction::CatchRet:
            case llvm::Instruction::CatchSwitch:
            case llvm::Instruction::CallBr:
            case llvm::Instruction::Add:
            case llvm::Instruction::Sub:
            case llvm::Instruction::Mul:
            case llvm::Instruction::UDiv:
            case llvm::Instruction::SDiv:
            case llvm::Instruction::URem:
            case llvm::Instruction::SRem:
            case llvm::Instruction::FRem:
            case llvm::Instruction::Shl:
            case llvm::Instruction::LShr:
            case llvm::Instruction::AShr:
            case llvm::Instruction::And:
            case llvm::Instruction::Or:
            case llvm::Instruction::Xor:
            case llvm::Instruction::Alloca:
            case llvm::Instruction::Load:
            case llvm::Instruction::Store:
            case llvm::Instruction::GetElementPtr:
            case llvm::Instruction::Fence:
            case llvm::Instruction::AtomicCmpXchg:
            case llvm::Instruction::AtomicRMW:
            case llvm::Instruction::Trunc:
            case llvm::Instruction::ZExt:
            case llvm::Instruction::SExt:
            case llvm::Instruction::FPToUI:
            case llvm::Instruction::FPToSI:
            case llvm::Instruction::UIToFP:
            case llvm::Instruction::SIToFP:
            case llvm::Instruction::PHI:
            case llvm::Instruction::Select:
            case llvm::Instruction::UserOp1:
            case llvm::Instruction::UserOp2:
            case llvm::Instruction::VAArg:
            case llvm::Instruction::ExtractElement:
            case llvm::Instruction::InsertElement:
            case llvm::Instruction::ShuffleVector:
            case llvm::Instruction::ExtractValue:
            case llvm::Instruction::InsertValue:
            case llvm::Instruction::LandingPad:
            case llvm::Instruction::Freeze:
            default:
                llvm::outs() << "Unhandled Instruction:" << I << "\n";
                exit(1);  // NOLINT
        }
    }
}
