#ifndef CIRE_GRAPH_H
#define CIRE_GRAPH_H

#include "Node.h"

class Graph {
private:
public:
  std::map<VariableNode*, ibex::Interval*> inputs;
  std::vector<Node *> outputs;

  Graph() = default;
  ~Graph() = default;

  virtual void write(std::ostream &os) const;

  // Prints string representation of this node
  friend std::ostream& operator<<(std::ostream& os, const Graph &graph);
};

#endif //CIRE_GRAPH_H
