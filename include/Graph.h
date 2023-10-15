#ifndef CIRE_GRAPH_H
#define CIRE_GRAPH_H

#include "Node.h"

class Graph {
private:
public:
  std::map<string, FreeVariable *> inputs;
  std::vector<string> outputs;
  std::map<string, Node *> variables;
  std::set<Node *> nodes;

  std::set<Node *> workList;
  std::set<Node *> derivativeComputedNodes;

  Graph() = default;
  ~Graph() = default;

  virtual void write(std::ostream &os) const;

  // Prints string representation of this node
  friend std::ostream& operator<<(std::ostream& os, const Graph &graph);

  void generateExprDriver();
  void generateExpr(Node *node);
  void generateErrExprDriver();
  void generateErrExpr(Node *node);

  Node *findFreeVarNode(string Var) const;
  Node *findVarNode(string Var) const;
};

#endif //CIRE_GRAPH_H
