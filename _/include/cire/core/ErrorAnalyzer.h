#ifndef CIRE_ERRORANALYZER_H
#define CIRE_ERRORANALYZER_H

#include "cire/core/InstructionMetadata.h"
#include "cire/core/Node.hpp"
#include "cire/interfaces/IBEXInterface.h"

#include <vector>

// Forward declarations
class ErrorAnalysisResult;
struct InstructionErrorInfo;

class ErrorAnalyzer {
public:
    unsigned int errorExpressionOperatorThreshold = 10000;
    std::map<ir::Node*, unsigned> nodeNumOptCallsMap;

    // Data structures for derivative computation
    // Map from node to number of parents of node
    std::map<ir::Node*, unsigned long> numParentsOfNode;
    // Nodes to compute derivative for
    std::set<ir::Node*> workList;

    // Nodes to compute derivative for
    std::set<ir::Node*> nextWorkList;
    // Map with derivative information (Contains maps of derivatives of expression corresponding to
    // the node corresponding key in the inner map with respect to node corresponding key in outer
    // map)
    std::map<ir::Node*, std::map<ir::Node*, ibex::ExprNode*>> bwdDerivatives;
    // Map with type cast rounding information (Contains maps of type cast amount to operation
    // corresponding the node corresponding key in the outer map)
    std::map<ir::Node*, std::map<ir::Node*, ibex::ExprNode*>> typeCastRnd;
    // Map from depth to nodes at that depth whose Backward derivative has been computed
    std::map<int, std::set<ir::Node*>> derivativeComputedNodes;
    // Map from depth to nodes at that depth whose error has been computed
    std::map<int, std::set<ir::Node*>> errorComputedNodes;
    // Map of node from parents of node
    std::map<ir::Node*, std::set<ir::Node*>> parentsOfNode;

    std::map<ir::Node*, ibex::ExprNode*> errAccumulator;

    std::map<ir::Node*, std::vector<std::pair<ir::Node*, ibex::ExprNode*>>> perInstructionErrors;

    ErrorAnalyzer();

    bool parentsVisited(ir::Node* node);

    void derivativeComputingDriver();
    void derivativeComputing(ir::Node* node);

    void errorComputingDriver(const std::set<ir::Node*>& candidate_nodes, IBEXInterface* ibexInterface);
    void errorComputing(ir::Node* node, IBEXInterface* ibexInterface);

    void propagateError(ir::Node* node, IBEXInterface* ibexInterface);

    void printBwdDerivative(ir::Node* outNode, ir::Node* WRTNode);
    void printBwdDerivativesIbexExprs();

    void logBwdDerivative(ir::Node* outNode, ir::Node* WRTNode);
    void logBwdDerivativesIbexExprs();

    std::map<ir::Node*, std::vector<InstructionErrorInfo>> getInstructionErrorBreakdown(IBEXInterface* ibexInterface,
                                                                                        class Graph* graph = nullptr);
};

ibex::ExprNode* getDerivativeWRTChildNode(ir::Node* node, int index);

template<class KeyType, class ValType>
std::vector<KeyType> keys(std::map<KeyType, ValType> map);

template<class KeyType, class ValType>
bool contains(std::map<KeyType, ValType> map, KeyType key);

template<class KeyType, class ValType>
ValType findWithDefaultInsertion(std::map<KeyType, ValType> map, KeyType key, ValType defaultVal);

#endif  // CIRE_ERRORANALYZER_H
