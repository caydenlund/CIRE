#ifndef CIRE_ERRORANALYZER_H
#define CIRE_ERRORANALYZER_H

#include "cire/core/Node.h"
#include "cire/interfaces/IBEXInterface.h"

struct InstructionErrorInfo;
class ErrorAnalysisResult;

class ErrorAnalyzer {
public:
    unsigned int errorExpressionOperatorThreshold = 10000;
    std::map<Node*, unsigned> nodeNumOptCallsMap;

    // Data structures for derivative computation
    // Map from node to number of parents of node
    std::map<Node*, unsigned long> numParentsOfNode;
    // Nodes to compute derivative for
    std::set<Node*> workList;

    // Nodes to compute derivative for
    std::set<Node*> nextWorkList;
    // Map with derivative information (Contains maps of derivatives of expression corresponding to
    // the node corresponding key in the inner map with respect to node corresponding key in outer
    // map)
    std::map<Node*, std::map<Node*, ibex::ExprNode*>> bwdDerivatives;
    // Map with type cast rounding information (Contains maps of type cast amount to operation
    // corresponding the node corresponding key in the outer map)
    std::map<Node*, std::map<Node*, ibex::ExprNode*>> typeCastRnd;
    // Map from depth to nodes at that depth whose Backward derivative has been computed
    std::map<int, std::set<Node*>> derivativeComputedNodes;
    // Map from depth to nodes at that depth whose error has been computed
    std::map<int, std::set<Node*>> errorComputedNodes;
    // Map of node from parents of node
    std::map<Node*, std::set<Node*>> parentsOfNode;

    std::map<Node*, ibex::ExprNode*> errAccumulator;
    
    std::map<Node*, std::vector<std::pair<Node*, ibex::ExprNode*>>> perInstructionErrors;
    
    std::map<int, std::pair<std::string, std::string>> llvmInstructionInfo;

    ErrorAnalyzer();

    bool parentsVisited(Node* node);

    void derivativeComputingDriver();
    void derivativeComputing(Node* node);

    void errorComputingDriver(const std::set<Node*>& candidate_nodes, IBEXInterface* ibexInterface);
    void errorComputing(Node* node, IBEXInterface* ibexInterface);

    void propagateError(Node* node, IBEXInterface* ibexInterface);

    void printBwdDerivative(Node* outNode, Node* WRTNode);
    void printBwdDerivativesIbexExprs();

    void logBwdDerivative(Node* outNode, Node* WRTNode);
    void logBwdDerivativesIbexExprs();
    
    std::map<Node*, std::vector<InstructionErrorInfo>> getInstructionErrorBreakdown(IBEXInterface* ibexInterface);
};

ibex::ExprNode* getDerivativeWRTChildNode(Node* node, int index);

template<class KeyType, class ValType>
std::vector<KeyType> keys(std::map<KeyType, ValType> map);

template<class KeyType, class ValType>
bool contains(std::map<KeyType, ValType> map, KeyType key);

template<class KeyType, class ValType>
ValType findWithDefaultInsertion(std::map<KeyType, ValType> map, KeyType key, ValType defaultVal);

#endif  // CIRE_ERRORANALYZER_H
