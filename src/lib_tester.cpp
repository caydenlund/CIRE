#include <iostream>
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

  ibex::Array<const ExprSymbol> *symbols1;
  symbols1 = new ibex::Array<const ExprSymbol>;
  symbols1->add(a);
  symbols1->add(b);

  Function f1(*symbols1, a+b);
  cout << "f1=" << f1.eval(m) << endl;


  symbols1 = new ibex::Array<const ExprSymbol>;
  symbols1->add(a);
  symbols1->add(b);

  Function f2(*symbols1, a-b);
  cout << "f2=" << f2.eval(m) << endl;

  return 0;
}