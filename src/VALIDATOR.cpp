#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Function.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/AsmParser/Parser.h>

#include <iostream>
#include <llvm/IRReader/IRReader.h>
#include <cmath>

// Function to create higher precision shadow instructions from a function to another function
void createShadowFunction(llvm::Function *srcFunction, llvm::Function *destFunction) {
  llvm::LLVMContext &context = srcFunction->getContext();
  llvm::IRBuilder<> builder(context);

  // Create a mapping of arguments from the source function to the destination function
  llvm::ValueToValueMapTy VMap;
  for (auto &arg : srcFunction->args()) {
    llvm::Argument *newArg = &*destFunction->arg_begin() + arg.getArgNo();
    VMap[&arg] = newArg;
  }

  // Clone the basic blocks of the source function to the destination function
  for (auto &srcBlock : *srcFunction) {
    llvm::BasicBlock *destBlock = llvm::BasicBlock::Create(context, srcBlock.getName(), destFunction);
    VMap[&srcBlock] = destBlock;


    // Clone the instructions of the source basic block to the destination basic block
    for (auto &srcInst : srcBlock) {
      llvm::Value *clonedInst;
      switch (srcInst.getOpcode()) {
        case llvm::Instruction::FAdd:
        case llvm::Instruction::FSub:
        case llvm::Instruction::FMul:
        case llvm::Instruction::FDiv:
        case llvm::Instruction::FRem: {
          clonedInst = llvm::BinaryOperator::Create(
                  static_cast<llvm::Instruction::BinaryOps>(srcInst.getOpcode()), VMap[srcInst.getOperand(0)], VMap[srcInst.getOperand(1)], "", destBlock);
          }
          break;
        case llvm::Instruction::FPExt: {
          if (srcInst.getType() == builder.getHalfTy()) {
            clonedInst = builder.CreateFPExt(VMap[srcInst.getOperand(0)], builder.getFloatTy());
          } else if (srcInst.getType() == builder.getFloatTy()) {
            clonedInst = builder.CreateFPExt(VMap[srcInst.getOperand(0)], builder.getDoubleTy());
          } else {
            std::cerr << "Unsupported type for FPExt instruction!" << std::endl;
            srcInst.dump();
            exit(1);
          }
          break;
        }
        case llvm::Instruction::FPTrunc: {
          if (srcInst.getType() == builder.getDoubleTy()) {
            clonedInst = builder.CreateFPTrunc(VMap[srcInst.getOperand(0)], builder.getFloatTy());
          } else if (srcInst.getType() == builder.getFloatTy()) {
            clonedInst = builder.CreateFPTrunc(VMap[srcInst.getOperand(0)], builder.getHalfTy());
          } else {
            std::cerr << "Unsupported type for FPTrunc instruction!" << std::endl;
            srcInst.dump();
            exit(1);
          }
          break;
        }
        case llvm::Instruction::Call: {
          auto CalledFunction = llvm::cast<llvm::CallInst>(&srcInst)->getCalledFunction();
          std::vector<llvm::Value *> args;
          for (unsigned i = 0; i < llvm::cast<llvm::CallInst>(&srcInst)->arg_size(); i++) {
            args.push_back(VMap[llvm::cast<llvm::CallInst>(&srcInst)->getArgOperand(i)]);
          }

          if (CalledFunction->isIntrinsic()) {
            if (CalledFunction->getName().contains("f32")) {
              std::string intrinsicName = CalledFunction->getName().str();
              intrinsicName.replace(intrinsicName.find("f32"), 3, "f64");
              llvm::Function *intrinsicFunc = destFunction->getParent()->getFunction(intrinsicName);
              clonedInst = builder.CreateCall(intrinsicFunc, args);
            } else {
              std::cerr << "Unsupported intrinsic function!" << std::endl;
            }
          } else {
            std::cerr << "Unsupported call instruction!" << std::endl;
            srcInst.dump();
            exit(1);
          }
        }
        case llvm::Instruction::Ret: {
          clonedInst = llvm::ReturnInst::Create(context, VMap[llvm::cast<llvm::ReturnInst>(&srcInst)->getReturnValue()], destBlock);
          break;
        }
        default:
          break;
      }
      builder.Insert(clonedInst);
      VMap[&srcInst] = clonedInst;
    }
  }

}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <input LLVM IR file>" << std::endl;
    return 1;
  }

  // Initialize LLVM context and create a new module
  llvm::LLVMContext context;
  llvm::SMDiagnostic err;

  // Parse the input LLVM IR file
  std::unique_ptr<llvm::Module> inputModule = llvm::parseIRFile(argv[1], err, context);
  if (!inputModule) {
    std::cerr << "Error reading IR file: " << argv[1] << std::endl;
    err.print(argv[0], llvm::errs());
    return 1;
  }

  // Extract the first function from the input module
  llvm::Function *firstFunction = nullptr;
  for (auto &func : *inputModule) {
    if (!func.isDeclaration()) {
      firstFunction = &func;
      break;
    }
  }

  if (!firstFunction) {
    std::cerr << "No non-declaration functions found in the input module!" << std::endl;
    return 1;
  }

  // Create a new module
  auto newModule = std::make_unique<llvm::Module>("main_module", context);
  llvm::IRBuilder<> builder(context);

  // Create the main function
  llvm::FunctionType *mainFuncType = llvm::FunctionType::get(builder.getInt32Ty(), false);
  llvm::Function *mainFunc = llvm::Function::Create(
          mainFuncType, llvm::Function::ExternalLinkage, "main", *newModule);

  // Create a basic block for the main function
  llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "entry", mainFunc);
  builder.SetInsertPoint(entryBlock);


  // Loop creation
  llvm::BasicBlock *loopCond = llvm::BasicBlock::Create(context, "loop.cond", mainFunc);
  llvm::BasicBlock *loopBody = llvm::BasicBlock::Create(context, "loop.body", mainFunc);
  llvm::BasicBlock *loopEnd = llvm::BasicBlock::Create(context, "loop.end", mainFunc);

  // Define the start, end, and step values
  llvm::Value *start = llvm::ConstantFP::get(context, llvm::APFloat(1.0f)); // Example: 1.0
  llvm::Value *end = llvm::ConstantFP::get(context, llvm::APFloat(2.0f));   // Example: 2.0

  // Declare external nextafterf function
  llvm::FunctionType *nextafterFuncType = llvm::FunctionType::get(builder.getFloatTy(),
                                                                  {builder.getFloatTy(), builder.getFloatTy()},
                                                                  false);
  llvm::Function *nextafterf = llvm::Function::Create(nextafterFuncType,
                                                      llvm::Function::ExternalLinkage,
                                                      "nextafterf", newModule.get());

  // Allocate space for the current loop variable
  llvm::Value *current = builder.CreateAlloca(builder.getFloatTy(), nullptr, "current");
  builder.CreateStore(start, current);
  builder.CreateBr(loopCond);

  // Loop condition block
  builder.SetInsertPoint(loopCond);
  llvm::Value *currentVal = builder.CreateLoad(builder.getFloatTy(), current, "currentVal");
  llvm::Value *cond = builder.CreateFCmpOLE(currentVal, end, "cond"); // Compare current <= end
  builder.CreateCondBr(cond, loopBody, loopEnd);

  // Loop body block
  builder.SetInsertPoint(loopBody);
  // Add your loop body operations here

  llvm::Type *retType;
  std::vector<llvm::Type*> argTypes;

  // Get new return type
  if(firstFunction->getReturnType()->isHalfTy()) {
    retType = builder.getFloatTy();
  } else if(firstFunction->getReturnType()->isFloatTy()) {
    retType = builder.getDoubleTy();
  } else {
    std::cerr << "Unsupported return type ";
    firstFunction->getReturnType()->print(llvm::errs());
    std::cerr << " for Validator!" << std::endl;
    return 1;
  }

  // Create new argument types
  for(auto &arg : firstFunction->args()) {
    if(arg.getType()->isHalfTy()) {
      argTypes.push_back(builder.getFloatTy());
    } else if(arg.getType()->isFloatTy()) {
      argTypes.push_back(builder.getDoubleTy());
    } else {
      std::cerr << "Unsupported argument type " << arg.getType() << " for Validator!" << std::endl;
      return 1;
    }
  }

  // Create a type similar to firstFunction but with floating-point types of a higher precision
  llvm::FunctionType *shadowFunctionType = llvm::FunctionType::get(retType,argTypes,false);

  // Clone the first function into the new module
  llvm::ValueToValueMapTy VMap;
  llvm::Function *shadowFunction = llvm::Function::Create(shadowFunctionType,
                                                          llvm::Function::ExternalLinkage,
                                                          "shadow_" + firstFunction->getName(), newModule.get());

  createShadowFunction(firstFunction, shadowFunction);

  llvm::Function *clonedFunction = llvm::CloneFunction(firstFunction, VMap);
  clonedFunction->removeFromParent();
  newModule->getFunctionList().push_back(clonedFunction);
  // Prepare arguments for the call to the cloned function
  std::vector<llvm::Value*> args;
  for (auto &arg : firstFunction->args()) {
    if (arg.getType()->isIntegerTy()) {
      args.push_back(builder.getInt32(0));  // Example: pass 0 for integer arguments
    } else if (arg.getType()->isFloatTy()) {
      args.push_back(llvm::ConstantFP::get(context, llvm::APFloat(1.1f)));  // Example: pass 0.0 for float
    } else if (arg.getType()->isDoubleTy()) {
      args.push_back(llvm::ConstantFP::get(context, llvm::APFloat(1.1)));  // Example: pass 0.0 for double
    } else {
      std::cerr << "Unsupported argument type!" << std::endl;
      return 1;
    }
  }

  std::vector<llvm::Value*> shadowArgs;
  for (auto &arg : shadowFunction->args()) {
    if (arg.getType()->isIntegerTy()) {
      shadowArgs.push_back(builder.getInt32(0));  // Example: pass 0 for integer arguments
    } else if (arg.getType()->isFloatTy()) {
      shadowArgs.push_back(llvm::ConstantFP::get(context, llvm::APFloat(1.1f)));  // Example: pass 0.0 for float
    } else if (arg.getType()->isDoubleTy()) {
      shadowArgs.push_back(llvm::ConstantFP::get(context, llvm::APFloat(1.1)));  // Example: pass 0.0 for double
    } else {
      std::cerr << "Unsupported argument type!" << std::endl;
      return 1;
    }
  }

  // FPExt the return value of the original function to the higher precision
  llvm::Value *originalFuncCallExt = builder.CreateFPExt(builder.CreateCall(clonedFunction, args), retType);
  llvm::Value *shadowFuncCall = builder.CreateCall(shadowFunction, shadowArgs);

  // Take the difference of the two function calls
  llvm::Value *final = builder.CreateFSub(originalFuncCallExt, shadowFuncCall);

  // Declare C standard library printf
  llvm::FunctionType *printfType = llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(context),
                                                           llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(context)),
                                                           true);
  auto printfFunc = newModule->getOrInsertFunction("printf", printfType);

  auto* formatStr = llvm::ConstantDataArray::getString(context, "%f\n", true);
  auto* globalVar = new llvm::GlobalVariable(
          *newModule,
          formatStr->getType(),
          true, // Constant
          llvm::GlobalValue::PrivateLinkage,
          formatStr,
          ".str"
  );
  llvm::Value* fmtStr = llvm::ConstantExpr::getPointerCast(globalVar,
                                                           llvm::PointerType::getUnqual(llvm::IntegerType::get(context, 8)));

  // Ensure the doubleValue is properly typed as a variadic argument
  llvm::Value* doubleAsArg = builder.CreateFPExt(final, llvm::Type::getDoubleTy(context));

  // Call printf
  builder.CreateCall(printfFunc, {fmtStr, doubleAsArg}, "printfCall");

  // Call nextafterf to calculate the next representable value
  llvm::Value *nextVal = builder.CreateCall(nextafterf, {currentVal, end}, "nextVal");
  builder.CreateStore(nextVal, current);
  builder.CreateBr(loopCond);

  // Return 0 in main
  // Loop end block
  builder.SetInsertPoint(loopEnd);
  builder.CreateRet(builder.getInt32(0));

  // Verify the main function and new module
  if (llvm::verifyFunction(*mainFunc, &llvm::errs())) {
    std::cerr << "Function verification failed!" << std::endl;
    return 1;
  }
  if (llvm::verifyModule(*newModule, &llvm::errs())) {
    std::cerr << "Module verification failed!" << std::endl;
    return 1;
  }

  // Output the new module to a file
  std::error_code EC;
  llvm::raw_fd_ostream dest("output.ll", EC, llvm::sys::fs::OF_None);
  if (EC) {
    llvm::errs() << "Could not open file: " << EC.message();
    return 1;
  }
  newModule->print(dest, nullptr);

  std::cout << "Successfully created output.ll with main function calling the cloned function." << std::endl;
  return 0;
}
