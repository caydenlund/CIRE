//
// Created by tanmay on 10/2/23.
//

#ifndef CIRE_CIRE_H
#define CIRE_CIRE_H

#include<string>
#include<map>
#include "Graph.h"

// Conducting the whole scanning and parsing of Calc++.
class CIRE {
public:
  Graph *graph;
  // The name of the file being parsed.
  std::string file;
  bool abstraction = false;
  std::pair<unsigned int, unsigned int> abstractionWindow =
          std::make_pair(10, 40);

  CIRE();
  ~CIRE();

  void setFile(std::string file);
  void setAbstaction(bool value);
  void setAbstractionWindow(std::pair<unsigned int, unsigned int> window);
  void setMinDepth(unsigned int depth);
  void setMaxDepth(unsigned int depth);
  std::map<Node *, ibex::IntervalVector> performErrorAnalysis();
};

#endif //CIRE_CIRE_H
