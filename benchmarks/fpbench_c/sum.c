#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double x0, double x1, double x2) {
	double p0 = (x0 + x1) - x2;
	double p1 = (x1 + x2) - x0;
	double p2 = (x2 + x0) - x1;
	return (p0 + p1) + p2;
}

