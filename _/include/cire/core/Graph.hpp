#ifndef CIRE_GRAPH_H
#define CIRE_GRAPH_H

#include "cire/core/ErrorAnalyzer.h"
#include "cire/core/SourceMapper.h"
#include "cire/core/SymbolTable.h"

#include <memory>

namespace llvm {
    class Value;
}  // namespace llvm

class ErrorAnalysisResult {
public:
    ibex::Interval outputExtrema;
    ibex::Interval errorExtrema;
    ibex::IntervalVector optPoint;
    double totalOptimizationTime;
    unsigned int numOptimizationCalls;
};

class Graph {
public:
    bool concretizeErrorComps = false;
    bool collectErrorCompData = false;
    std::map<int, SymbolTable*> symbolTables;
    int currentScope = 0;
    // Connects a variable name to a FreeVariable representing an interval
    std::map<string, ir::FreeVariable*> inputs;
    // List of output variables
    std::vector<string> outputs;
    // List of nodes ever created. Used for cleaning up memory
    std::set<ir::Node*> nodes;
    // Depth table contains a map from depth to a set of nodes at that depth
    std::map<int, std::set<ir::Node*>> depthTable;
    unsigned int numOperatorsOutput = 0;

    ErrorAnalyzer* errorAnalyzer = new ErrorAnalyzer();
    std::map<ir::Node*, ErrorAnalysisResult> errorAnalysisResults;
    IBEXInterface* ibexInterface = new IBEXInterface();

    std::string validationFile;

    // Instance-level LLVM IR to CIRE node mapping (replaces global maps)
    std::map<llvm::Value*, ir::Node*> llvmValueToNode;
    std::map<ir::Node*, llvm::Value*> nodeToLLVMValue;

    // Source mapper for debug information extraction
    std::unique_ptr<SourceMapper> sourceMapper = std::make_unique<SourceMapper>();

    // Instruction type index for fast queries
    std::map<std::string, std::vector<ir::Node*>> instructionTypeIndex;

    std::map<unsigned int, std::map<std::string, unsigned int>> abstractionMetrics;

    Graph() = default;
    virtual ~Graph();

    void setValidationFile(std::string _validationFile);

    // LLVM IR mapping methods
    void registerLLVMNode(llvm::Value* llvmValue, ir::Node* node);
    ir::Node* getNodeByLLVMValue(llvm::Value* llvmValue) const;
    llvm::Value* getLLVMValueByNode(ir::Node* node) const;

    // Instruction query methods
    [[nodiscard]] std::vector<ir::Node*> getNodesByInstructionType(const std::string& type) const;
    void indexNodeByInstructionType(ir::Node* node, const std::string& type);

    virtual void write(std::ostream& out) const;

    // Prints string representation of this node
    friend std::ostream& operator<<(std::ostream& os, const Graph& graph);

    void createNewSymbolTable();

    void generateIbexSymbols();

    [[nodiscard]] ir::Node* findFreeVarNode(string Var) const;
    [[nodiscard]] ir::Node* findVarNode(string Var) const;

    void setupDerivativeComputation(std::set<ir::Node*> candidate_nodes);
    void generateExprDriver(const std::set<ir::Node*>& candidate_nodes);
    void generateExpr(ir::Node* node, std::map<int, std::set<ir::Node*>>& generatedExprsAtDepth,
                      std::map<ibex::ExprNode*, std::set<ir::Node*>>& cseTable);

    // Merges node1 and node2 into node2
    ir::Node* mergeNodes(ir::Node* node1, ir::Node* node2, std::map<ir::Node*, std::set<ir::Node*>>& parentsOfNode);

    // Concretizes the error components - Backward Derivatives and Ab
    void concretizeErrorComponents();

    // Collects BwdDerivative and Local Error data
    void examineBwdDerivativeAndLocalError();

    bool compareDAGs(ibex::ExprNode expr1, ibex::ExprNode expr2);

    // Abstraction related functions
    std::set<ir::Node*> flattenSubDags(ir::Node* node, unsigned int min_depth, unsigned int max_depth);
    std::set<ir::Node*> findCommonNodes(ir::Node* node, unsigned int min_depth, unsigned int max_depth);
    std::map<ir::Node*, std::set<ir::Node*>> findCommonDependencies(std::set<ir::Node*> node, unsigned int min_depth,
                                                                    unsigned int max_depth);
    std::set<ir::Node*> filterNodesWithOperationWithinDepth(ir::Node::OpType op, unsigned int max_depth);
    std::set<ir::Node*> filterCandidatesForAbstraction(unsigned int max_depth, unsigned int lower_bound,
                                                       unsigned int upper_bound);
    std::pair<unsigned int, std::set<ir::Node*>>
    selectNodesForAbstraction(unsigned int max_depth, unsigned int bound_min_depth, unsigned int bound_max_depth);
    void performAbstraction(unsigned int bound_min_depth, unsigned int bound_max_depth);

    void findOutputExtrema(const std::set<ir::Node*>& candidate_nodes);
    void findErrorExtrema(const std::set<ir::Node*>& candidate_nodes);

    std::map<ir::Node*, ErrorAnalysisResult> simplifyWithAbstraction(const std::set<ir::Node*>& candidate_nodes,
                                                                     unsigned max_depth, bool isFinal = false);

    std::vector<ir::Node*> modProbeList();
    void abstractNodes(std::map<ir::Node*, std::vector<ibex::Interval>> results);
    void rebuildAst();
    void rebuildAstNode(ir::Node* node, std::map<ir::Node*, unsigned int>& completed);

    // Run the parser on file F.  Return 0 on success.
    int parse(const char& f);
};

// Give Flex the prototype of yylex we want ...
#define YY_DECL int yylex(Graph* graph)
// ... and declare it for the parser's sake.
YY_DECL;

#endif  // CIRE_GRAPH_H
