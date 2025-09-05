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

using namespace llvm;
using namespace std;

// Define the global maps declared as extern in the header
std::map<llvm::Value*, Node*> llvmToCireNodeMap;
std::map<Node*, llvm::Value*> cireToLLVMNodeMap;

void addDataForCreatedNode(Instruction& instr, Graph& graph, Node* node) {
    if (instr.getType()->isHalfTy()) {
        node->setRoundingType(Node::RoundingType::FL16);
        node->setRoundingFromType(Node::RoundingType::FL16);
    } else if (instr.getType()->isFloatTy()) {
        node->setRoundingType(Node::RoundingType::FL32);
        node->setRoundingFromType(Node::RoundingType::FL32);
    } else if (instr.getType()->isDoubleTy()) {
        node->setRoundingType(Node::RoundingType::FL64);
        node->setRoundingFromType(Node::RoundingType::FL64);
    }
    graph.nodes.insert(node);
    graph.depthTable[node->depth].insert(node);
    graph.numOperatorsOutput++;

    llvmToCireNodeMap[&instr] = node;
    cireToLLVMNodeMap[node] = &instr;
    graph.symbolTables[graph.currentScope]->insert(instr.getName().str(), node);
}

Node* getNodeFromLLVMValue(Value* val, Graph& graph) {
    if (isa<ConstantData>(val)) {
        if (val->getType()->isFloatingPointTy()) {
            auto* CD = dyn_cast<ConstantFP>(val);
            auto* new_node = new Double(CD->getValueAPF().convertToDouble());

            graph.nodes.insert(new_node);
            llvmToCireNodeMap[val] = new_node;
            cireToLLVMNodeMap[new_node] = val;
            return new_node;
        } else if (val->getType()->isIntegerTy()) {
            auto* CI = dyn_cast<ConstantInt>(val);
            auto* new_node = new Integer(CI->getSExtValue());

            graph.nodes.insert(new_node);
            llvmToCireNodeMap[val] = new_node;
            cireToLLVMNodeMap[new_node] = val;
            return new_node;
        }
    } else {
        return llvmToCireNodeMap[val];
    }

    return nullptr;
}

void parseInputsInLLVM(Graph& graph, Function& func) {
    graph.createNewSymbolTable();

    // Iterate the function arguments
    for (auto& arg : func.args()) {
        Type* arg_type = arg.getType();
        Node::RoundingType rounding_type;

        // Getting the CIRE type of argument corresponding the LLVM type
        switch (arg_type->getTypeID()) {
            case Type::TypeID::IntegerTyID: {
                rounding_type = Node::RoundingType::INT;
                break;
            }
            case Type::TypeID::HalfTyID: {
                rounding_type = Node::RoundingType::FL16;
                break;
            }
            case Type::TypeID::FloatTyID: {
                rounding_type = Node::RoundingType::FL32;
                break;
            }
            case Type::TypeID::DoubleTyID: {
                rounding_type = Node::RoundingType::FL64;
                break;
            }
            default: {
                outs() << "Unhandled Type:" << arg_type << "\n";
                exit(1);
            }
        }

        auto* new_variable = new VariableNode(rounding_type);
        graph.nodes.insert(new_variable);
        graph.symbolTables[graph.currentScope]->insert(arg.getNameOrAsOperand().c_str(), new_variable);
        graph.inputs[arg.getNameOrAsOperand().c_str()] = new FreeVariable(rounding_type);
        graph.nodes.insert(graph.inputs[arg.getNameOrAsOperand().c_str()]);
    }
}

void parseExprsInLLVM(Graph& graph, Function& func) {
    // TODO: Make this work for multiple Basic blocks after you have support for
    //  conditionals
    // assume we have just one basic block
    assert(func.size() == 1 && "Function has more than one basic block. Cant work.");

    // Get the basic block
    BasicBlock& BB = func.getEntryBlock();

    // Iterate over the instructions in the basic block
    for (auto& I : BB) {
        auto opcode = I.getOpcode();

        switch (opcode) {
            case Instruction::Ret: {
                if (I.getOperand(0)->getType()->isFloatingPointTy()) {
                    graph.outputs.push_back(I.getOperand(0)->getName().str());
                }
                break;
            }
            case Instruction::Br:
            case Instruction::Switch:
            case Instruction::IndirectBr:
            case Instruction::Invoke:
            case Instruction::Resume:
            case Instruction::Unreachable:
            case Instruction::CleanupRet:
            case Instruction::CatchRet:
            case Instruction::CatchSwitch: {
                outs() << "Unhandled Instruction:" << I << "\n";
                exit(1);
            }
            case Instruction::CallBr: {
                outs() << "Unhandled Instruction:" << I << "\n";
                exit(1);
            }
            case Instruction::FNeg: {
                if (I.getType()->isFloatingPointTy()) {
                    auto op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    addDataForCreatedNode(I, graph, &(-*op1));
                }
                break;
            }
            case Instruction::Add: {
                outs() << "Unhandled Instruction:" << I << "\n";
                exit(1);
            }
            case Instruction::FAdd: {
                if (I.getType()->isFloatingPointTy()) {
                    auto op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    auto op2 = getNodeFromLLVMValue(I.getOperand(1), graph);

                    addDataForCreatedNode(I, graph, &(*op1 + op2));
                }
                break;
            }
            case Instruction::Sub: {
                outs() << "Unhandled Instruction:" << I << "\n";
                exit(1);
            }
            case Instruction::FSub: {
                if (I.getType()->isFloatingPointTy()) {
                    auto op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    auto op2 = getNodeFromLLVMValue(I.getOperand(1), graph);

                    addDataForCreatedNode(I, graph, &(*op1 - op2));
                }
                break;
            }
            case Instruction::Mul: {
                outs() << "Unhandled Instruction:" << I << "\n";
                exit(1);
            }
            case Instruction::FMul: {
                if (I.getType()->isFloatingPointTy()) {
                    auto op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    auto op2 = getNodeFromLLVMValue(I.getOperand(1), graph);

                    addDataForCreatedNode(I, graph, &(*op1 * op2));
                }
                break;
            }
            case Instruction::UDiv:
            case Instruction::SDiv: {
                outs() << "Unhandled Instruction:" << I << "\n";
                exit(1);
            }
            case Instruction::FDiv: {
                if (I.getType()->isFloatingPointTy()) {
                    auto op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    auto op2 = getNodeFromLLVMValue(I.getOperand(1), graph);

                    addDataForCreatedNode(I, graph, &(*op1 / op2));
                }
                break;
            }
            case Instruction::URem:
            case Instruction::SRem:
            case Instruction::FRem:
            case Instruction::Shl:
            case Instruction::LShr:
            case Instruction::AShr:
            case Instruction::And:
            case Instruction::Or:
            case Instruction::Xor:
            case Instruction::Alloca:
            case Instruction::Load:
            case Instruction::Store:
            case Instruction::GetElementPtr:
            case Instruction::Fence:
            case Instruction::AtomicCmpXchg:
            case Instruction::AtomicRMW:
            case Instruction::Trunc:
            case Instruction::ZExt:
            case Instruction::SExt:
            case Instruction::FPToUI:
            case Instruction::FPToSI:
            case Instruction::UIToFP:
            case Instruction::SIToFP: {
                outs() << "Unhandled Instruction:" << I << "\n";
                exit(1);
            }
            case Instruction::FPTrunc:
                if (I.getType()->isFloatingPointTy()) {
                    auto op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    Node::RoundingType rounding_type;
                    if (dyn_cast<FPTruncInst>(&I)->getDestTy()->isFloatTy()) {
                        rounding_type = Node::FL32;
                    } else if (dyn_cast<FPTruncInst>(&I)->getDestTy()->isDoubleTy()) {
                        rounding_type = Node::FL64;
                    } else {
                        outs() << "Incorrect Destination type:" << I.getType() << "\n";
                        exit(1);
                    }

                    addDataForCreatedNode(I, graph, &fptrunc(*op1, rounding_type));
                }
                break;
            case Instruction::FPExt:
                if (I.getType()->isFloatingPointTy()) {
                    auto op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    Node::RoundingType rounding_type;
                    if (dyn_cast<FPExtInst>(&I)->getDestTy()->isFloatTy()) {
                        rounding_type = Node::FL32;
                    } else if (dyn_cast<FPExtInst>(&I)->getDestTy()->isDoubleTy()) {
                        rounding_type = Node::FL64;
                    } else {
                        outs() << "Incorrect Destination type:" << I.getType() << "\n";
                        exit(1);
                    }

                    addDataForCreatedNode(I, graph, &fpext(*op1, rounding_type));
                }
                break;
            case Instruction::PtrToInt:
            case Instruction::IntToPtr:
            case Instruction::BitCast:
            case Instruction::AddrSpaceCast:
            case Instruction::ICmp:
            case Instruction::FCmp:
            case Instruction::PHI:
                outs() << "Unhandled Instruction:" << I << "\n";
                exit(1);
            case Instruction::Call: {
                auto CI = dyn_cast<CallInst>(&I);
                if (CI->getCalledFunction()->arg_size() == 1) {
                    auto CalledFunctionName = CI->getCalledFunction()->getName().str();
                    auto op1 = getNodeFromLLVMValue(CI->getOperand(0), graph);

                    if (CalledFunctionName == "sin" || CalledFunctionName == "sinf"
                        || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::sin) {
                        addDataForCreatedNode(I, graph, &sin(*op1));
                    } else if (CalledFunctionName == "cos" || CalledFunctionName == "cosf"
                               || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::cos) {
                        addDataForCreatedNode(I, graph, &cos(*op1));
                    } else if (CalledFunctionName == "tan" || CalledFunctionName == "tanf"
                               || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::tan) {
                        addDataForCreatedNode(I, graph, &tan(*op1));
                    } else if (CalledFunctionName == "sinh" || CalledFunctionName == "sinhf"
                               || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::sinh) {
                        addDataForCreatedNode(I, graph, &sinh(*op1));
                    } else if (CalledFunctionName == "cosh" || CalledFunctionName == "coshf"
                               || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::cosh) {
                        addDataForCreatedNode(I, graph, &cosh(*op1));
                    } else if (CalledFunctionName == "tanh" || CalledFunctionName == "tanhf"
                               || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::tanh) {
                        addDataForCreatedNode(I, graph, &tanh(*op1));
                    } else if (CalledFunctionName == "asin" || CalledFunctionName == "asinf"
                               || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::asin) {
                        addDataForCreatedNode(I, graph, &asin(*op1));
                    } else if (CalledFunctionName == "acos" || CalledFunctionName == "acosf"
                               || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::acos) {
                        addDataForCreatedNode(I, graph, &acos(*op1));
                    } else if (CalledFunctionName == "atan" || CalledFunctionName == "atanf"
                               || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::atan) {
                        addDataForCreatedNode(I, graph, &atan(*op1));
                    } else if (CalledFunctionName == "log" || CalledFunctionName == "logf"
                               || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::log) {
                        addDataForCreatedNode(I, graph, &log(*op1));
                    } else if (CalledFunctionName == "sqrt" || CalledFunctionName == "sqrtf"
                               || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::sqrt) {
                        addDataForCreatedNode(I, graph, &sqrt(*op1));
                    } else if (CalledFunctionName == "exp" || CalledFunctionName == "expf"
                               || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::exp) {
                        addDataForCreatedNode(I, graph, &exp(*op1));
                    } else {
                        outs() << "Unhandled Function in Call Instruction:" << I << "\n";
                        exit(1);
                    }
                } else if (CI->getCalledFunction()->arg_size() == 3) {
                    auto CalledFunctionName = CI->getCalledFunction()->getName().str();
                    auto op1 = getNodeFromLLVMValue(I.getOperand(0), graph);
                    auto op2 = getNodeFromLLVMValue(I.getOperand(1), graph);
                    auto op3 = getNodeFromLLVMValue(I.getOperand(2), graph);

                    if (CI->getCalledFunction()->getIntrinsicID() == Intrinsic::fma
                        || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::fmuladd) {
                        addDataForCreatedNode(I, graph, &fma(*op1, *op2, *op3));
                    } else {
                        outs() << "Unhandled Function in Call Instruction:" << I << "\n";
                        exit(1);
                    }
                } else {
                    outs() << "Function with " << CI->getCalledFunction()->arg_size()
                           << " arguments in Call Instruction not handled:" << I << "\n";
                    exit(1);
                }

                break;
            }
            case Instruction::Select:
            case Instruction::UserOp1:
            case Instruction::UserOp2:
            case Instruction::VAArg:
            case Instruction::ExtractElement:
            case Instruction::InsertElement:
            case Instruction::ShuffleVector:
            case Instruction::ExtractValue:
            case Instruction::InsertValue:
            case Instruction::LandingPad:
            case Instruction::Freeze: {
                outs() << "Unhandled Instruction:" << I << "\n";
                exit(1);
            }
        }
    }
}
