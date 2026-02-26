double cast_test(double x, double y) {
  double z = x + y;
  float final = x - z;
  return final;
}

int main() {
  double x = 1.0;
  double y = 2.0;
  double result = cast_test(x, y);
  return 0;
}