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

  // Data structures for derivative computation
  // Map from node to number of parents of node
  std::map<Node *, int> numParentsOfNode;
  // Nodes to compute derivative for
  std::set<Node *> workList;
  // Map with derivative information (Contains maps of derivatives of expression corresponding to the node corresponding
  // key in the inner map with respect to node corresponding key in outer map)
  std::map<Node *, std::map<Node *, ibex::ExprNode *>> BwdDerivatives;
  // Map from depth to nodes at that depth. Used to check if all nodes at a depth have been processed
  std::map<int, std::set<Node *>> derivativeComputedNodes;

  Graph() = default;
  ~Graph() = default;

  virtual void write(std::ostream &os) const;

  // Prints string representation of this node
  friend std::ostream& operator<<(std::ostream& os, const Graph &graph);

  void generateExprDriver();
  void generateExpr(Node *node);

  bool parentsVisited(Node *node);

  void generateErrExprDriver();
  void generateErrExpr(Node *node);

  Node *findFreeVarNode(string Var) const;
  Node *findVarNode(string Var) const;
};

#endif //CIRE_GRAPH_H
