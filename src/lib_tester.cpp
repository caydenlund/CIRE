#include <iostream>
#include "ibex.h"

using namespace std;
using namespace ibex;

int main(int argc, char * argv[]) {
  Interval x(0, 1.0), y(0, 1.0);
//  auto a = Variable("x");
//  auto b = Variable("y");
  string x_str = "x";
  string y_str = "y";
  string z_str = "z";
  const ExprNode *a = &ExprSymbol::new_(x_str.c_str());
  const ExprNode *b = &ExprSymbol::new_(x_str.c_str());
  const ExprNode *c = &ExprSymbol::new_(z_str.c_str());

  IntervalVector m(x);
  m.resize(m.size()+1);
  m[m.size()-1] = y;

  cout << (*a**a).simplify(3) << endl;
  cout << "x+y = " << x + y << endl;
  cout << "m = " << m << endl;

//  for(int i = 0; i < 2; i++) {
////    Array<const ExprSymbol> symbols = Array<const ExprSymbol>();
//    Array<const ExprSymbol> *symbols = new ibex::Array<const ibex::ExprSymbol>;
//    symbols->add(*(ExprSymbol *)a);
//    symbols->add(*(ExprSymbol *)b);
//
//      Function f(*symbols, *a+*b);
//      cout << "f=" << f.eval(m) << endl;
////      delete symbols;
//
//    Array<const ExprSymbol> symbols1 = Array<const ExprSymbol>();
//    symbols1.add(*(ExprSymbol *)a);
//    symbols1.add(*(ExprSymbol *)b);
//      Function g(symbols1, *a-*b);
//      cout << "g=" << g.eval(m).mag() << endl;
//  }
  // Create an ibex function and evaluate it
//  Array<const ExprSymbol> symbols = Array<const ExprSymbol>();
//  symbols.add(a);
//  symbols.add(b);
//
//  Function f(symbols, a+b);
//  cout << "f=" << f.eval(m) << endl;
//
//  Array<const ExprSymbol> symbols = Array<const ExprSymbol>();
//  symbols->add(a);
//  symbols->add(b);

//  Function g(symbols, a-b);
//  cout << "g=" << g.eval(m) << endl;

//  ibex::Array<const ExprSymbol> *symbols1;
//  symbols1 = new ibex::Array<const ExprSymbol>;
//  symbols1->add(a);
//  symbols1->add(b);

//  Function f1(*symbols1, (a* pow(2, -53)+b* pow(2, -53)) * pow(2, -53));
//  cout << "f1=" << f1.eval(m) << endl;

//  SystemFactory factory;
//  factory.add_var(a);
//  factory.add_var(b);
//  factory.add_goal((a* pow(2, -53)+b* pow(2, -53)) * pow(2, -53));
//  factory.add_goal((a+b));
//  System sys(factory);
//  DefaultOptimizer opt(sys);
//  opt.optimize(m);
//  opt.report();



  return 0;
}