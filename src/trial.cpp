#include <iostream>
#include "Node.h"

using namespace std;
using namespace ibex;

namespace trial {
  int main(int argc, char *argv[]) {
    Interval x(1, 3), y(5, 7);

    cout << "x+y = " << x + y << endl;
    return 0;
  }
}