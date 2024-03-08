#ifndef CIRE_GRAPH_H
#define CIRE_GRAPH_H

#include "SymbolTable.h"
#include "ErrorAnalyzer.h"
#include "IBEXInterface.h"
#include "Logging.h"

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

  ErrorAnalyzer *errorAnalyzer = new ErrorAnalyzer();
  IBEXInterface *ibexInterface = new IBEXInterface();

  std::string validationFile;

  Graph() = default;
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
  void errorComputingDriver(std::set<Node*> candidate_nodes);
  void generateExprDriver(std::set<Node *> candidate_nodes);
  void generateExpr(Node *node);

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

  std::map<Node *, ibex::Interval> FindOutputExtrema(const std::set<Node *>& candidate_nodes);
  std::map<Node *, ibex::Interval> FindErrorExtrema(const std::set<Node *>& candidate_nodes);

  std::map<Node *, std::vector<ibex::Interval>> SimplifyWithAbstraction(const std::set<Node*>& candidate_nodes, unsigned max_depth, bool isFinal=false);

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
