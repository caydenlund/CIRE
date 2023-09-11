#include <iostream>
#include "Node.h"
#include "trial.cpp"

using namespace std;
using namespace ibex;

namespace cire {
  int main(int argc, char * argv[]) {
    Interval x(0, 1), y(2, 3);

    cout << "x+y = " << x + y << endl;
    return 0;
  }
}

int main(int argc, char * argv[]) {
  if(argc > 1 && string(argv[1]) == "trial")
    return trial::main(argc, argv);
  else
    return cire::main(argc, argv);
}