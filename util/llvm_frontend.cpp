#include "llvm_frontend.h"

using namespace llvm;

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
      case Instruction::Ret:
      case Instruction::Br:
      case Instruction::Switch:
      case Instruction::IndirectBr:
      case Instruction::Invoke:
      case Instruction::Resume:
      case Instruction::Unreachable:
      case Instruction::CleanupRet:
      case Instruction::CatchRet:
      case Instruction::CatchSwitch:
      case Instruction::CallBr:{
        std::cout << "Unhandled class: Terminators" << std::endl;
        break;
      }
      case Instruction::FNeg: {
        std::cout << "Unhandled class: Unary operators" << std::endl;
        break;
      }
      case Instruction::Add:
      case Instruction::FAdd:
      case Instruction::Sub:
      case Instruction::FSub:
      case Instruction::Mul:
      case Instruction::FMul:
      case Instruction::UDiv:
      case Instruction::SDiv:
      case Instruction::FDiv:
      case Instruction::URem:
      case Instruction::SRem:
      case Instruction::FRem: {
        std::cout << "Unhandled class: Binary operators" << std::endl;
        break;
      }
      case Instruction::Shl:
      case Instruction::LShr:
      case Instruction::AShr:
      case Instruction::And:
      case Instruction::Or:
      case Instruction::Xor: {
        std::cout << "Unhandled class: Bitwise operators" << std::endl;
        break;
      }
      case Instruction::Alloca:
      case Instruction::Load:
      case Instruction::Store:
      case Instruction::GetElementPtr:
      case Instruction::Fence:
      case Instruction::AtomicCmpXchg:
      case Instruction::AtomicRMW: {
        std::cout << "Unhandled class: Memory operators" << std::endl;
        break;
      }
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
      case Instruction::AddrSpaceCast: {
        std::cout << "Unhandled class: Cast operators" << std::endl;
        break;
      }
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
        std::cout << "Unhandled class: Other operators" << std::endl;
        break;
      }
    }

  }

}