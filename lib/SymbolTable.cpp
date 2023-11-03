#include "../include/SymbolTable.h"

void SymbolTable::insert(string symbol, Node *node) {
//  std::cout << "Inserting " << symbol << " in scope " << scopeID << std::endl;
//  std::cout << *node << std::endl;
  table[symbol] = node;
}

Node *SymbolTable::lookup(string symbol, Node *node) {
  auto it = table.find(symbol);
  if (it != table.end()) {
    return it->second;
  }
  return node;
}
