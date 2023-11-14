#ifndef CIRE_ERRORANALYZER_H
#define CIRE_ERRORANALYZER_H

#include "Node.h"

class ErrorAnalyzer {
public:
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
  // Map from depth to nodes at that depth whose error has been computed
  std::map<int, std::set<Node *>> errorComputedNodes;

  std::map<Node *, ibex::ExprNode *> ErrAccumulator;

  bool parentsVisited(Node *node);

  void derivativeComputingDriver();
  void derivativeComputing(Node *node);

  void errorComputing(Node *node);

  void propagateError(Node *node);

  void printBwdDerivative(Node *outNode, Node *WRTNode);
  void printBwdDerivativesIbexExprs();
};

ibex::ExprNode *getDerivativeWRTChildNode(Node *node, int index);

template<class T1, class T2>
std::vector<T1> keys(std::map<T1, T2> map);
template<class T1, class T2>
T2 findWithDefaultInsertion(std::map<T1, T2> map, T1 key, T2 defaultVal);

#endif //CIRE_ERRORANALYZER_H
