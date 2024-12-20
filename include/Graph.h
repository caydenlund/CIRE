#ifndef CIRE_GRAPH_H
#define CIRE_GRAPH_H

#include "SymbolTable.h"
#include "ErrorAnalyzer.h"
#include "IBEXInterface.h"

class ErrorAnalysisResult {
public:
  ibex::Interval outputExtrema;
  ibex::Interval errorExtrema;
  ibex::IntervalVector lbPoint;
  ibex::IntervalVector ubPoint;
  double totalOptimizationTime;
};

class Graph {
private:
public:
  Logging log;
  unsigned int debugLevel = 0;
  unsigned int logLevel = 0;
  std::map<int, SymbolTable *> symbolTables;
  int currentScope = 0;
  // Connects a variable name to a FreeVariable representing an interval
  std::map<string, FreeVariable *> inputs;
  // List of output variables
  std::vector<string> outputs;
  // List of nodes ever created. Used for cleaning up memory
  std::set<Node *> nodes;
  // Depth table contains a map from depth to a set of nodes at that depth
  std::map<int, std::set<Node *>> depthTable;
  unsigned int numOperatorsOutput = 0;

  ErrorAnalyzer *errorAnalyzer = new ErrorAnalyzer(&log);
  std::map<Node*, ErrorAnalysisResult> errorAnalysisResults;
  IBEXInterface *ibexInterface = new IBEXInterface();

  std::string validationFile;

  std::map<unsigned int, std::map<std::string, unsigned int>> abstractionMetrics;

  Graph() = default;
  explicit Graph(string logFile);
  ~Graph();

  void setValidationFile(std::string _validationFile);

  virtual void write(std::ostream &os) const;

  // Prints string representation of this node
  friend std::ostream& operator<<(std::ostream& os, const Graph &graph);

  void createNewSymbolTable();

  void generateIbexSymbols();

  Node *findFreeVarNode(string Var) const;
  Node *findVarNode(string Var) const;

  void setupDerivativeComputation(std::set<Node*> candidate_nodes);
  void errorComputingDriver(const std::set<Node*> &candidate_nodes);
  void generateExprDriver(const std::set<Node *> &candidate_nodes);
  void generateExpr(Node *node, std::map<int, std::set<Node *>> &generatedExprsAtDepth, std::map<ibex::ExprNode *, std::set<Node *>> &cseTable);

  bool compareDAGs(ibex::ExprNode expr1, ibex::ExprNode expr2);

  // Abstraction related functions
  std::set<Node*> FlattenSubDAGS(Node* node, unsigned int min_depth, unsigned int max_depth);
  std::set<Node*> FindCommonNodes(Node* node, unsigned int min_depth, unsigned int max_depth);
  std::map<Node*, std::set<Node*>> FindCommonDependencies(std::set<Node*> node, unsigned int min_depth, unsigned int max_depth);
  std::set<Node*> FilterNodesWithOperationWithinDepth(Node::Op op, unsigned int max_depth);
  std::set<Node*> FilterCandidatesForAbstraction(unsigned int max_depth,
                                                    unsigned int lower_bound,
                                                    unsigned int upper_bound);
  std::pair<unsigned int, std::set<Node*>> selectNodesForAbstraction(unsigned int max_depth,
                                             unsigned int bound_min_depth,
                                             unsigned int bound_max_depth);
  void performAbstraction(unsigned int bound_min_depth, unsigned int bound_max_depth);

  void FindOutputExtrema(const std::set<Node *>& candidate_nodes);
  void FindErrorExtrema(const std::set<Node *>& candidate_nodes);

  std::map<Node *, ErrorAnalysisResult> SimplifyWithAbstraction(const std::set<Node*>& candidate_nodes, unsigned max_depth, bool isFinal=false);

  std::vector<Node *> ModProbeList();
  void AbstractNodes(std::map<Node *, std::vector<ibex::Interval>> results);
  void RebuildAST();
  void RebuildASTNode(Node *node, std::map<Node *, unsigned int> &completed);

  // Run the parser on file F.  Return 0 on success.
  int parse(const char &f);
};

// Give Flex the prototype of yylex we want ...
#define YY_DECL int yylex(Graph *graph)
// ... and declare it for the parser's sake.
YY_DECL;

#endif //CIRE_GRAPH_H
