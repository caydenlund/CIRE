#include <iostream>
#include <cmath>

using namespace std;

double f(double x) {
  return sin(x);
}

int main() {
  cout << f(2.0) << endl;
  return 0;
}