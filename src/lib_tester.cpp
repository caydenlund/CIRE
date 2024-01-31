#include <iostream>
#include <ibex.h>
#include <ibex_Expr.h>

using namespace std;
using namespace ibex;

int main(int argc, char * argv[]) {
  Interval x(1, 3), y(5, 7);
  auto a = Variable("x");
  auto b = Variable("y");
  auto a_s = a.symbol;
  auto b_s = b.symbol;
  cout << a+b << endl;

  IntervalVector m(x);
  m.resize(m.size()+1);
  m[m.size()-1] = y;

  cout << "x+y = " << x + y << endl;
  cout << "m = " << m << endl;

  ibex::Array<const ExprSymbol> symbols;
  symbols.add(*a.symbol);
  symbols.add(*b.symbol);

  auto exp1 = *a.symbol+*b.symbol;
  Function f1(symbols, exp1);
  cout << "f1=" << f1.eval(m) << endl;


  a = Variable("b");
  b = Variable("c");
  symbols[0] = *a.symbol;
  symbols[1] = *b.symbol;

  auto exp2 = *a.symbol-*b.symbol;
  Function f2(symbols, *a.symbol-*b.symbol);
  cout << "f2=" << f2.eval(m) << endl;

  return 0;
}