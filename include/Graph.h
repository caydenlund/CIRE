#ifndef CIRE_GRAPH_H
#define CIRE_GRAPH_H

#include "SymbolTable.h"
#include "ErrorAnalyzer.h"
#include "IBEXInterface.h"

class Graph {
private:
public:
  std::map<int, SymbolTable *> symbolTables;
  int currentScope = 0;
  // Connects a variable name to a FreeVariable representing an interval
  std::map<string, FreeVariable *> inputs;
  // List of output variables
  std::vector<string> outputs;
  // List of nodes ever created. Used for cleaning up memory
  std::set<Node *> nodes;

  ErrorAnalyzer *errorAnalyzer = new ErrorAnalyzer();
  IBEXInterface *ibexInterface = new IBEXInterface();

  Graph() = default;
  ~Graph();

  virtual void write(std::ostream &os) const;

  // Prints string representation of this node
  friend std::ostream& operator<<(std::ostream& os, const Graph &graph);

  void createNewSymbolTable();

  Node *findFreeVarNode(string Var) const;
  Node *findVarNode(string Var) const;

  void setupDerivativeComputation();
  void errorComputingDriver();
  void generateExprDriver();
  void generateExpr(Node *node);

  // Run the parser on file F.  Return 0 on success.
  int parse(const char &f);

};

// Give Flex the prototype of yylex we want ...
#define YY_DECL int yylex(Graph *graph)
// ... and declare it for the parser's sake.
YY_DECL;

#endif //CIRE_GRAPH_H
