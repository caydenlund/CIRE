#include <iostream>

double func(double a, double b, double c) {
  double d = a + (b + c);
  return d;
}

int main() {
  std::cout << func(3.5, 2.5, 4.5) << std::endl;
  return 0;
}