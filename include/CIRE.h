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

  CIRE();
};

#endif //CIRE_CIRE_H
