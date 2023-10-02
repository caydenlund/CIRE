#include <iostream>
#include "Graph.h"

using namespace std;
using namespace ibex;

namespace trial {
  int main(int argc, char *argv[]) {

    Interval x(1, 3), y(5, 7);
    IntervalVector m(x);
    m.resize(m.size()+1);
    m[m.size()-1] = y;

    cout << "x+y = " << x + y << endl;
    cout << "m = " << m << endl;

    Graph g;
    g.inputs.


    return 0;
  }
}