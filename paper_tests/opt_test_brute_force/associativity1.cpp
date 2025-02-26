#include <iostream>
#include <iomanip>
#include <quadmath.h>
#include "chrono"

int main() {
  unsigned precision = 20;
  double myDouble;

//  float delta = 1.1920928955078125 * 1e-7;
  double delta = 2.220446049250313080847263336181640625 * 1e-16;

  std::cout << "Enter a double value: ";
  std::cin >> myDouble;
  __float128 myQuad = myDouble;
  __float128 err = 0;
  unsigned long long highest_err_at = 0;

  std::cout << "You entered (double): " << std::fixed << std::setprecision(precision) << myDouble << std::endl;
  std::cout << "You entered (quad):   " << std::fixed << std::setprecision(precision) << (__float80) myQuad << std::endl;

//  unsigned long long iterations = 4503599627370495;
  unsigned long long iterations = 8388607;
//  unsigned long long iterations = 10;

  const auto start = std::chrono::high_resolution_clock::now();

  for (unsigned long long i = 1; i < iterations; i++) {
    myDouble = myDouble + delta;
    myQuad = myDouble;
    double resDouble = myDouble + myDouble;
    __float128 resQuad = (__float128) (myQuad + myQuad);
    if(err < fabsq(resQuad - resDouble)) {
      err = fabsq(resQuad - resDouble);
      highest_err_at = i;
    }
//    std::cout << std::fixed << std::setprecision(precision) << myDouble << std::endl;
  }

  const auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;

  std::cout << "Final: " << std::fixed << std::setprecision(precision) << myDouble << std::endl;
  std::cout << "Error: " << std::fixed << std::setprecision(128) << (__float80) err << std::endl;
  std::cout << "Highest error at: " << highest_err_at << std::endl;
  std::cout << "Total Time taken: " << duration.count() << " seconds" << std::endl;
//  std::cout << "delta: " << std::fixed << std::setprecision(20) << delta << std::endl;
  return 0;
}