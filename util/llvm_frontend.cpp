#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include "llvm_frontend.h"

using namespace llvm;
using namespace std;

// Map from LLVM Values to CIRE Nodes
std::map<llvm::Value *, Node *> llvmToCireNodeMap;
std::map<Node *, llvm::Value *> cireToLLVMNodeMap;

void addDataForCreatedNode(Instruction &I, Graph &g, Node* res) {
  if (I.getType()->isHalfTy()) {
    res->setRoundingFromType(Node::RoundingType::FL16);
  } else if (I.getType()->isFloatTy()) {
    res->setRoundingFromType(Node::RoundingType::FL32);
  } else if (I.getType()->isDoubleTy()) {
    res->setRoundingFromType(Node::RoundingType::FL64);
  }
  g.nodes.insert(res);
  g.depthTable[res->depth].insert(res);

  llvmToCireNodeMap[&I] = res;
  cireToLLVMNodeMap[res] = &I;
  g.symbolTables[g.currentScope]->insert(I.getNameOrAsOperand(), res);
}

Node *getNodeFromLLVMValue(Value *V, Graph &g) {
  if(isa<ConstantData>(V)) {
    if(V->getType()->isFloatingPointTy()) {
      auto *CD = dyn_cast<ConstantFP>(V);
      auto new_node = new Double(CD->getValueAPF().convertToDouble());

      g.nodes.insert(new_node);
      llvmToCireNodeMap[V] = new_node;
      cireToLLVMNodeMap[new_node] = V;
      return new_node;
    } else if(V->getType()->isIntegerTy()) {
      auto *CI = dyn_cast<ConstantInt>(V);
      auto new_node = new Integer(CI->getSExtValue());

      g.nodes.insert(new_node);
      llvmToCireNodeMap[V] = new_node;
      cireToLLVMNodeMap[new_node] = V;
      return new_node;
    }
  } else {
    return llvmToCireNodeMap[V];
  }

  return nullptr;
}

void parseInputsInLLVM(Graph &g, Function &F) {
  g.createNewSymbolTable();

  // Iterate the function arguments
  for (auto &arg : F.args()) {
    Type *arg_type = arg.getType();
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

    auto *new_variable = new VariableNode(rounding_type);
    g.nodes.insert(new_variable);
    g.symbolTables[g.currentScope]->insert(arg.getNameOrAsOperand(), new_variable);
    g.inputs[arg.getNameOrAsOperand()] = new FreeVariable(rounding_type);
    g.nodes.insert(g.inputs[arg.getNameOrAsOperand()]);
  }
}

void parseExprsInLLVM(Graph &g, Function &F) {
  // TODO: Make this work for multiple Basic blocks after you have support for
  //  conditionals
  // assume we have just one basic block
  assert(F.size() == 1 && "Function has more than one basic block. Cant work.");

  // Get the basic block
  BasicBlock &BB = F.getEntryBlock();

  // Iterate over the instructions in the basic block
  for (auto &I : BB) {
    auto opcode = I.getOpcode();

    switch(opcode) {
      case Instruction::Ret: {
        if (I.getOperand(0)->getType()->isFloatingPointTy()) {
          g.outputs.push_back(I.getOperand(0)->getNameOrAsOperand());
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
          auto op1 = getNodeFromLLVMValue(I.getOperand(0), g);
          addDataForCreatedNode(I, g, &(-*op1));
        }
        break;
      }
      case Instruction::Add: {
        outs() << "Unhandled Instruction:" << I << "\n";
        exit(1);
      }
      case Instruction::FAdd: {
        if (I.getType()->isFloatingPointTy()) {
          auto op1 = getNodeFromLLVMValue(I.getOperand(0), g);
          auto op2 = getNodeFromLLVMValue(I.getOperand(1), g);

          addDataForCreatedNode(I, g, &(*op1+op2));
        }
        break;
      }
      case Instruction::Sub: {
        outs() << "Unhandled Instruction:" << I << "\n";
        exit(1);
      }
      case Instruction::FSub: {
        if (I.getType()->isFloatingPointTy()) {
          auto op1 = getNodeFromLLVMValue(I.getOperand(0), g);
          auto op2 = getNodeFromLLVMValue(I.getOperand(1), g);

          addDataForCreatedNode(I, g, &(*op1-op2));
        }
        break;
      }
      case Instruction::Mul: {
        outs() << "Unhandled Instruction:" << I << "\n";
        exit(1);
      }
      case Instruction::FMul: {
        if (I.getType()->isFloatingPointTy()) {
          auto op1 = getNodeFromLLVMValue(I.getOperand(0), g);
          auto op2 = getNodeFromLLVMValue(I.getOperand(1), g);

          addDataForCreatedNode(I, g, &(*op1*op2));
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
          auto op1 = getNodeFromLLVMValue(I.getOperand(0), g);
          auto op2 = getNodeFromLLVMValue(I.getOperand(1), g);

          addDataForCreatedNode(I, g, &(*op1/op2));
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
      case Instruction::SIToFP:
      case Instruction::FPTrunc:
      case Instruction::FPExt:
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
        if(CI->getCalledFunction()->arg_size() == 1) {
          auto CalledFunctionName = CI->getCalledFunction()->getNameOrAsOperand();
          auto op1 = getNodeFromLLVMValue(CI->getOperand(0), g);

          if(CalledFunctionName == "sin" ||
            CalledFunctionName == "sinf" || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::sin) {
            addDataForCreatedNode(I, g, &sin(*op1));
          } else if(CalledFunctionName == "cos" ||
                    CalledFunctionName == "cosf" || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::cos) {
            addDataForCreatedNode(I, g, &cos(*op1));
          } else if(CalledFunctionName == "tan" ||
                    CalledFunctionName == "tanf" || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::tan) {
            addDataForCreatedNode(I, g, &tan(*op1));
          } else if(CalledFunctionName == "sinh" ||
                    CalledFunctionName == "sinhf" || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::sinh) {
            addDataForCreatedNode(I, g, &sinh(*op1));
          } else if(CalledFunctionName == "cosh" ||
                    CalledFunctionName == "coshf" || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::cosh) {
            addDataForCreatedNode(I, g, &cosh(*op1));
          } else if(CalledFunctionName == "tanh" ||
                    CalledFunctionName == "tanhf" || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::tanh) {
            addDataForCreatedNode(I, g, &tanh(*op1));
          } else if(CalledFunctionName == "asin" ||
                    CalledFunctionName == "asinf" || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::asin) {
            addDataForCreatedNode(I, g, &asin(*op1));
          } else if(CalledFunctionName == "acos" ||
                    CalledFunctionName == "acosf" || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::acos) {
            addDataForCreatedNode(I, g, &acos(*op1));
          } else if(CalledFunctionName == "atan" ||
                    CalledFunctionName == "atanf" || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::atan) {
            addDataForCreatedNode(I, g, &atan(*op1));
          } else if(CalledFunctionName == "log" ||
                    CalledFunctionName == "logf" || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::log) {
            addDataForCreatedNode(I, g, &log(*op1));
          } else if(CalledFunctionName == "sqrt" ||
                    CalledFunctionName == "sqrtf" || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::sqrt) {
            addDataForCreatedNode(I, g, &sqrt(*op1));
          } else if(CalledFunctionName == "exp" ||
                    CalledFunctionName == "expf" || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::exp) {
            addDataForCreatedNode(I, g, &exp(*op1));
          } else {
            outs() << "Unhandled Function in Call Instruction:" << I << "\n";
            exit(1);
          }
        } else if(CI->getCalledFunction()->arg_size() == 3) {
          auto CalledFunctionName = CI->getCalledFunction()->getNameOrAsOperand();
          auto op1 = getNodeFromLLVMValue(I.getOperand(0), g);
          auto op2 = getNodeFromLLVMValue(I.getOperand(1), g);
          auto op3 = getNodeFromLLVMValue(I.getOperand(2), g);

          if(CI->getCalledFunction()->getIntrinsicID() == Intrinsic::fma
          || CI->getCalledFunction()->getIntrinsicID() == Intrinsic::fmuladd) {
            addDataForCreatedNode(I, g, &fma(*op1, *op2, *op3));
          } else {
            outs() << "Unhandled Function in Call Instruction:" << I << "\n";
            exit(1);
          }
        } else {
          outs() << "Function with " << CI->getCalledFunction()->arg_size() << " arguments in Call Instruction not handled:"
          << I << "\n";
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