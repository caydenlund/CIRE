#include "CIRE_LLVM.h"

CIRE_LLVM::CIRE_LLVM() {
  graph = new Graph();
}

CIRE_LLVM::~CIRE_LLVM() {
  delete graph;
}

void CIRE_LLVM::parseInputFile(std::string file) {
  std::ifstream infile;

  infile.open(file);

  if (!infile.is_open()) {
    std::cout << "Error opening file " << file << std::endl;
    exit(1);
  }

  infile.close();
}