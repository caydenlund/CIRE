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
  std::map<Node *, unsigned long> numParentsOfNode;
  // Nodes to compute derivative for
  std::set<Node *> workList;

  // Nodes to compute derivative for
  std::set<Node *> nextWorkList;
  // Map with derivative information (Contains maps of derivatives of expression corresponding to the node corresponding
  // key in the inner map with respect to node corresponding key in outer map)
  std::map<Node *, std::map<Node *, ibex::ExprNode *>> BwdDerivatives;
  // Map from depth to nodes at that depth whose Backward derivative has been computed
  std::map<int, std::set<Node *>> derivativeComputedNodes;

  Graph() = default;
  ~Graph() = default;

  virtual void write(std::ostream &os) const;

  // Prints string representation of this node
  friend std::ostream& operator<<(std::ostream& os, const Graph &graph);

  Node *findFreeVarNode(string Var) const;
  Node *findVarNode(string Var) const;

  void generateExprDriver();
  void generateExpr(Node *node);

  bool parentsVisited(Node *node);

  void derivativeComputingDriver();
  void derivativeComputing(Node *node);

 void printBwdDerivativesIbexExprs();

};

ibex::ExprNode *getDerivativeWRTChildNode(Node *node, int index);

template<class T1, class T2>
std::vector<T1> keys(std::map<T1, T2> map);
template<class T1, class T2>
T2 findWithDefaultInsertion(std::map<T1, T2> map, T1 key, T2 defaultVal);


#endif //CIRE_GRAPH_H
