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

  void insert(const string& symbol, Node *node);
  Node *lookup(const string& symbol, Node *node = new VariableNode(Node::RoundingType::INT));
  string ReverseLookup(const Node *node);
};


#endif //CIRE_SYMBOLTABLE_H
