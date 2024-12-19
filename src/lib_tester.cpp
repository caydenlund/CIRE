#include <iostream>
#include "Graph.h"

using namespace std;
using namespace ibex;

int main(int argc, char * argv[]) {
  Interval x(0, 1.0), y(0, 1.0);
  auto a = Variable("x");
  auto b = Variable("y");

  IntervalVector m(x);
  m.resize(m.size()+1);
  m[m.size()-1] = y;

  cout << (a+b) << endl;
  cout << "x+y = " << x + y << endl;
  cout << "m = " << m << endl;

//  ibex::Array<const ExprSymbol> *symbols1;
//  symbols1 = new ibex::Array<const ExprSymbol>;
//  symbols1->add(a);
//  symbols1->add(b);

//  Function f1(*symbols1, (a* pow(2, -53)+b* pow(2, -53)) * pow(2, -53));
//  cout << "f1=" << f1.eval(m) << endl;

  SystemFactory factory;
  factory.add_var(a);
  factory.add_var(b);
//  factory.add_goal((a* pow(2, -53)+b* pow(2, -53)) * pow(2, -53));
  factory.add_goal((a+b));
  System sys(factory);
  DefaultOptimizer opt(sys);
  opt.optimize(m);
  opt.report();



  return 0;
}