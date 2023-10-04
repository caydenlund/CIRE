#include "../include/Graph.h"

Graph *graph;

std::ostream &operator<<(std::ostream &os, const Graph &graph) {
  graph.write(os);
  return os;
}

void Graph::write(std::ostream &os) const {
os << "Graph:" << std::endl;
  os << "Inputs:" << std::endl;
  for (auto &input : inputs) {
    os << "\t" << *input.first << " : " << *input.second << std::endl;
  }
  os << "Outputs:" << std::endl;
  for (auto &output : outputs) {
    os << "\t" << *output << std::endl;
  }
  os << "Variables:" << std::endl;
  for (auto &variable : variables) {
    os << "\t" << *variable.first << " : " << *variable.second << std::endl;
  }
}

Node *Graph::findVarNode(const ibex::ExprSymbol *Expr) const {
  auto it_inp = inputs.find(Expr);
  if (it_inp != inputs.end()) {
    return it_inp->second;
  }
  auto it_var = variables.find(Expr);
  if (it_var != variables.end()) {
    return it_var->second;
  }

  return nullptr;
}