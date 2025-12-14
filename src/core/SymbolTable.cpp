#include "cire/core/SymbolTable.h"

void SymbolTable::insert(const string& symbol, ir::Node* node) {
    //  std::cout << "Inserting " << symbol << " in scope " << scopeID << std::endl;
    //  std::cout << *node << std::endl;
    table[symbol] = node;
}

ir::Node* SymbolTable::lookup(const string& symbol, ir::Node* node) {
    auto it = table.find(symbol);
    if (it != table.end()) return it->second;
    return node;
}

/* Finds the symbol associated with the given node */
string SymbolTable::reverseLookup(const ir::Node* node) {
    for (auto& it : table) {
        if (it.second == node) return it.first;
    }
    return "";
}
