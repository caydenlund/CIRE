#include "Graph.h"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>

// Map from LLVM Values to CIRE Nodes
std::map<llvm::Value *, Node *> llvmToCireNodeMap;
std::map<Node *, llvm::Value *> cireToLLVMNodeMap;

void addDataForCreatedNode(llvm::Instruction &I, Graph &g, Node* res);
Node *getNodeFromLLVMValue(llvm::Value *V, Graph &g);
void parseInputsInLLVM(Graph &g, llvm::Function &F);
void parseExprsInLLVM(Graph &g, llvm::Function &F);
