#include <fenv.h>
#include <math.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0

double ex0(double t0, double w0, double N) {
	double h = 0.01;
	double L = 2.0;
	double m = 1.5;
	double g = 9.80665;
	double t = t0;
	double w = w0;
	double n = 0.0;
	int tmp = n < N;
	while (tmp) {
		double k1w = (-g / L) * sin(t);
		double k2t = w + ((h / 2.0) * k1w);
		double t_1 = t + (h * k2t);
		double k2w = (-g / L) * sin((t + ((h / 2.0) * w)));
		double w_2 = w + (h * k2w);
		double n_3 = n + 1.0;
		t = t_1;
		w = w_2;
		n = n_3;
		tmp = n < N;
	}
	return t;
}

