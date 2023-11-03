#ifndef CIRE_SYMBOLTABLE_H
#define CIRE_SYMBOLTABLE_H

#include "Node.h"

class SymbolTable {
public:
  static int SCOPE_COUNTER;
  int scopeID = SCOPE_COUNTER++;
  std::map<string, Node *> table;

  SymbolTable() = default;
  ~SymbolTable() = default;

  void insert(string symbol, Node *node);
  Node *lookup(string symbol, Node *node);
};


#endif //CIRE_SYMBOLTABLE_H
