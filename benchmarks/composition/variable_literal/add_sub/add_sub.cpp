#include <iostream>

using namespace std;

float f(float x, float y) {
  return x-3.9f+y;
}

int main() {
  cout << f(2, 3) << endl;
  return 0;
}