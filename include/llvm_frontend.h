#include "Graph.h"
#include "llvm/IR/Function.h"

void parseInputsInLLVM(Graph &g, llvm::Function &F);
void parseExprsInLLVM(Graph &g, llvm::Function &F);