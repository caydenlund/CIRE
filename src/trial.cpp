#include <iostream>
#include "Node.h"

using namespace std;
using namespace ibex;

namespace trial {
  int main(int argc, char *argv[]) {
    Interval x(1, 3), y(5, 7);

    cout << "x+y = " << x + y << endl;

    Float a(4.2);
    Float b(3.1);
    BinaryOp c(&a, &b);

    Node *temp = &c;

    cout << "a+b = " << *temp << endl;


    return 0;
  }
}