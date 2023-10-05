//
// Created by tanmay on 10/2/23.
//

#ifndef CIRE_CIRE_H
#define CIRE_CIRE_H

#include<string>
#include<map>
#include "parser.h"

// Give Flex the prototype of yylex we want ...
#define YY_DECL int yylex(Graph *graph)
// ... and declare it for the parser's sake.
YY_DECL;

// Conducting the whole scanning and parsing of Calc++.
class CIRE {
public:
  Graph *graph;
  // The name of the file being parsed.
  std::string file;

  CIRE();

  // Run the parser on file F.  Return 0 on success.
  int parse (const char &f);

};

#endif //CIRE_CIRE_H
