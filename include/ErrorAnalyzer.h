#ifndef CIRE_ERRORANALYZER_H
#define CIRE_ERRORANALYZER_H

#include "Node.h"
#include "Logging.h"
#include "IBEXInterface.h"

class ErrorAnalyzer {
public:
  unsigned int debugLevel = 0;
  unsigned int logLevel = 0;
  Logging &log;
  std::map<Node *, unsigned> nodeNumOptCallsMap;

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
  // Map with type cast rounding information (Contains maps of type cast amount to operation corresponding the node corresponding
  // key in the outer map)
  std::map<Node *, std::map<Node *, ibex::ExprNode *>> typeCastRnd;
  // Map from depth to nodes at that depth whose Backward derivative has been computed
  std::map<int, std::set<Node *>> derivativeComputedNodes;
  // Map from depth to nodes at that depth whose error has been computed
  std::map<int, std::set<Node *>> errorComputedNodes;
  // Map of node from parents of node
  std::map<Node *, std::set<Node *>> parentsOfNode;

  std::map<Node *, ibex::ExprNode *> ErrAccumulator;

  ErrorAnalyzer(Logging *log);

  bool parentsVisited(Node *node);

  void derivativeComputingDriver();
  void derivativeComputing(Node *node);

  void errorComputingDriver(const std::set<Node*> &candidate_nodes, IBEXInterface *ibexInterface);
  void errorComputing(Node *node, IBEXInterface *ibexInterface);

  void propagateError(Node *node, IBEXInterface *ibexInterface);

  void printBwdDerivative(Node *outNode, Node *WRTNode);
  void printBwdDerivativesIbexExprs();

  void logBwdDerivative(Node *outNode, Node *WRTNode);
  void logBwdDerivativesIbexExprs();
};

ibex::ExprNode *getDerivativeWRTChildNode(Node *node, int index);

template<class T1, class T2>
std::vector<T1> keys(std::map<T1, T2> map);

template<class T1, class T2>
bool contains(std::map<T1, T2> map, T1 key);

template<class T1, class T2>
T2 findWithDefaultInsertion(std::map<T1, T2> map, T1 key, T2 defaultVal);

#endif //CIRE_ERRORANALYZER_H
