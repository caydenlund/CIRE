#include <iostream>
#include <ibex.h>
#include "Graph.h"

using namespace std;
using namespace ibex;

int main(int argc, char * argv[]) {
  Interval x(1, 3), y(5, 7);
  auto a = Variable("x");
  auto b = Variable("y");
  cout << a+b << endl;

  IntervalVector m(x);
  m.resize(m.size()+1);
  m[m.size()-1] = y;

  cout << "x+y = " << x + y << endl;
  cout << "m = " << m << endl;

  Graph *graph = new Graph();
  graph->outputs.push_back(&ibex::ExprSymbol::new_("x"));

  return 0;
}