#include "llvm_frontend.h"

using namespace llvm;

// Map from LLVM Values to CIRE Nodes
std::map<llvm::Value *, Node *> llvmToCireNodeMap;

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
      case Instruction::CatchSwitch:
      case Instruction::CallBr: {
        outs() << "Unhandled Instruction:" << I << "\n";
        break;
      }
      case Instruction::FNeg: {
        if (I.getType()->isFloatingPointTy()) {
          auto op1 = llvmToCireNodeMap.find(I.getOperand(0))->second;
          auto res = &(-*op1);
          g.nodes.insert(res);

          llvmToCireNodeMap[&I] = res;
        }
        break;
      }
      case Instruction::Add: {
        outs() << "Unhandled Instruction:" << I << "\n";
        break;
      }
      case Instruction::FAdd: {
        if (I.getType()->isFloatingPointTy()) {
          auto op1 = llvmToCireNodeMap.find(I.getOperand(0))->second;
          auto op2 = llvmToCireNodeMap.find(I.getOperand(1))->second;
          auto res = &(*op1+*op2);
          g.nodes.insert(res);
          g.depthTable[res->depth].insert(res);

          llvmToCireNodeMap[&I] = res;
        }
        break;
      }
      case Instruction::Sub: {
        outs() << "Unhandled Instruction:" << I << "\n";
        break;
      }
      case Instruction::FSub: {
        if (I.getType()->isFloatingPointTy()) {
          auto op1 = llvmToCireNodeMap.find(I.getOperand(0))->second;
          auto op2 = llvmToCireNodeMap.find(I.getOperand(1))->second;
          auto res = &(*op1-*op2);
          g.nodes.insert(res);
          g.depthTable[res->depth].insert(res);

          llvmToCireNodeMap[&I] = res;
        }
        break;
      }
      case Instruction::Mul: {
        outs() << "Unhandled Instruction:" << I << "\n";
        break;
      }
      case Instruction::FMul: {
        if (I.getType()->isFloatingPointTy()) {
          auto op1 = llvmToCireNodeMap.find(I.getOperand(0))->second;
          auto op2 = llvmToCireNodeMap.find(I.getOperand(1))->second;
          auto res = &(*op1+*op2);
          g.nodes.insert(res);
          g.depthTable[res->depth].insert(res);

          llvmToCireNodeMap[&I] = res;
        }
        break;
      }
      case Instruction::UDiv:
      case Instruction::SDiv: {
        outs() << "Unhandled Instruction:" << I << "\n";
        break;
      }
      case Instruction::FDiv: {
        if (I.getType()->isFloatingPointTy()) {
          auto op1 = llvmToCireNodeMap.find(I.getOperand(0))->second;
          auto op2 = llvmToCireNodeMap.find(I.getOperand(1))->second;
          auto res = &(*op1+*op2);
          g.nodes.insert(res);
          g.depthTable[res->depth].insert(res);

          llvmToCireNodeMap[&I] = res;
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
      case Instruction::Call:
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
        break;
      }
    }

  }

}