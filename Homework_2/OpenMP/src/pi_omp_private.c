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

// Variables Declaration

uint32_t num_steps_private = 100000000;
double step;

// Internal function declaration

double pi_opm_private(uint32_t num_steps);


int main(int argc, char **argv) {
	num_steps_private = 100000000;
	pi_opm_private(num_steps_private);
	return 0;

}

// Declare main functions
double pi_opm_private(uint32_t num_steps){
	// Declare Internal Variables
	uint32_t i;
	double x, pi, sum = 0.0;
	double start_time, run_time;
	// Calcualte step size
	step = 1.0 / (double)num_steps;
	// get start time
	start_time = omp_get_wtime();
	// Make for loop parallel with the parallel construct
	#pragma omp parallel
	{
		// Set reduction pragma to optimize sum operation and make the x variable private between threads
		// the reduction operation makes the variable private and causes the system to perform a reduction optimization at the end of the parallel region
		#pragma omp for reduction(+:sum) private (x)
		for (i = 1; i <= num_steps; i++) {
			x = (i - 0.5) * step;
			sum = sum + 4.0 / (1.0 + x * x);
		}
	}
	// Calculates Pi
	pi = step * sum;
	run_time = omp_get_wtime() - start_time;
	// Prints Results
	printf("pi private reduction with %d steps is %lf in %lf seconds\n", num_steps, pi, run_time);
	return pi;
}
