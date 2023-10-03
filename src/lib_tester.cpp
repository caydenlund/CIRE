#include <iostream>
#include <ibex.h>

using namespace std;
using namespace ibex;

int main(int argc, char * argv[]) {
  Interval x(1, 3), y(5, 7);
  IntervalVector m(x);
  m.resize(m.size()+1);
  m[m.size()-1] = y;

  cout << "x+y = " << x + y << endl;
  cout << "m = " << m << endl;
  return 0;
}