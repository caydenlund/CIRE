#ifndef CIRE_SYMBOLTABLE_H
#define CIRE_SYMBOLTABLE_H

#include "cire/core/Node.hpp"

class SymbolTable {
public:
    static int SCOPE_COUNTER;
    int scopeID = SCOPE_COUNTER++;
    std::map<string, ir::Node*> table;

    SymbolTable() = default;
    ~SymbolTable() = default;

    void insert(const string& symbol, ir::Node* node);
    ir::Node* lookup(const string& symbol, ir::Node* node = new ir::VariableNode(ir::Node::RoundingType::INT));
    string reverseLookup(const ir::Node* node);
};


#endif  // CIRE_SYMBOLTABLE_H
