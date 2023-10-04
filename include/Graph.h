#ifndef CIRE_GRAPH_H
#define CIRE_GRAPH_H

#include "Node.h"

class Graph {
private:
public:
  std::map<const ibex::ExprSymbol *, FreeVariable *> inputs;
  std::vector<const ibex::ExprSymbol *> outputs;
  std::map<const ibex::ExprSymbol *, Node *> variables;

  Graph() = default;
  ~Graph() = default;

  virtual void write(std::ostream &os) const;

  // Prints string representation of this node
  friend std::ostream& operator<<(std::ostream& os, const Graph &graph);

  Node *findVarNode(const ibex::ExprSymbol *Expr) const;
};

#endif //CIRE_GRAPH_H
