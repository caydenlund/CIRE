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

  // Clone the first function into the new module
  llvm::ValueToValueMapTy VMap;
  llvm::Function *clonedFunction = llvm::CloneFunction(firstFunction, VMap);
  clonedFunction->removeFromParent();
  newModule->getFunctionList().push_back(clonedFunction);

  // Create the main function
  llvm::FunctionType *mainFuncType = llvm::FunctionType::get(builder.getInt32Ty(), false);
  llvm::Function *mainFunc = llvm::Function::Create(
          mainFuncType, llvm::Function::ExternalLinkage, "main", *newModule);

  // Create a basic block for the main function
  llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "entry", mainFunc);
  builder.SetInsertPoint(entryBlock);

  // Prepare arguments for the call to the cloned function
  std::vector<llvm::Value*> args;
  for (auto &arg : clonedFunction->args()) {
    if (arg.getType()->isIntegerTy()) {
      args.push_back(builder.getInt32(0));  // Example: pass 0 for integer arguments
    } else if (arg.getType()->isFloatTy()) {
      args.push_back(llvm::ConstantFP::get(context, llvm::APFloat(0.0f)));  // Example: pass 0.0 for float
    } else if (arg.getType()->isDoubleTy()) {
      args.push_back(llvm::ConstantFP::get(context, llvm::APFloat(0.0)));  // Example: pass 0.0 for double
    } else {
      std::cerr << "Unsupported argument type!" << std::endl;
      return 1;
    }
  }

  // Call the cloned function with the prepared arguments
  builder.CreateCall(clonedFunction, args);

  // Return 0 in main
  builder.CreateRet(builder.getInt32(0));

  // Verify the main function and new module
//  if (llvm::verifyFunction(*mainFunc, &llvm::errs())) {
//    std::cerr << "Function verification failed!" << std::endl;
//    return 1;
//  }
//  if (llvm::verifyModule(*newModule, &llvm::errs())) {
//    std::cerr << "Module verification failed!" << std::endl;
//    return 1;
//  }

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
