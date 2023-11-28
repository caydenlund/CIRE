#ifndef CIRE_CIRE_LLVM_H
#define CIRE_CIRE_LLVM_H

#include "Graph.h"

class CIRE_LLVM {
    Graph *graph;
public:
  CIRE_LLVM();
  ~CIRE_LLVM();

  void parseInputFile(std::string file);

};

#endif //CIRE_CIRE_LLVM_H
