/*
This program will numerically compute the integral of

                  4/(1+x*x)
from 0 to 1.

History: Written by Tim Mattson, 11/99.

Configure eclipse based: https://medium.com/swlh/openmp-on-ubuntu-1145355eeb2
*/

#include <omp.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

static long num_steps = 100000000;
double step;

// Implicit function declaration
double pi(uint32_t num_steps);

int main(int argc, char **argv) {
	num_steps = 100000000;
	pi(num_steps);
	return 0;
}

double pi(uint32_t num_steps){
	uint32_t i;
	double x, pi, sum = 0.0;
	double start_time, run_time;
	step = 1.0 / (double)num_steps;
	start_time = omp_get_wtime();

	for (i = 1; i <= num_steps; i++) {
		x = (i - 0.5) * step;
		sum = sum + 4.0 / (1.0 + x * x);
	}
	pi = step * sum;
	run_time = omp_get_wtime() - start_time;
	printf("pi with %d steps is %lf in %lf seconds\n", num_steps, pi, run_time);
	return pi;
}
