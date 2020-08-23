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

// Declare variables

uint32_t num_steps_teams = 100000000;
uint32_t num_threads = 0;
double step;

// Internal functions declarations

double pi_opm_threads(uint32_t num_steps, uint32_t num_threads);

// Main code

int main(int argc, char **argv) {
	num_steps_teams = 100000000;
	for (num_threads = 1; num_threads <= 8;num_threads++){
		printf("\n *** Starting Pi calculation with %d threads *** \n", num_threads);
		pi_opm_threads(num_steps_teams, num_threads);
	}
	return 0;
}

double pi_opm_threads(uint32_t num_steps, uint32_t requested_threads){
	// Define internal Variables
	double x, pi, sum  = 0.0;
	double start_time, run_time;
	step = 1.0 / (double)num_steps;
	start_time = omp_get_wtime();
	// Open MP Parallel pragma with num_threads
	#pragma omp parallel num_threads(requested_threads)
	{
		// Combine multiple threads with reduction for sum and and usi x variable private between threads
		#pragma omp for reduction(+:sum) private (x)
		for (uint32_t i = 1; i <= num_steps; i++){
			x = (i - 0.5) * step;
			sum = sum + 4.0 / (1.0 + x * x);
		}
	}
	// Calculate Pi Value
	pi = step * sum;
	run_time = omp_get_wtime() - start_time;
	// Print Results
	printf("pi implementation with parallel clause %d steps is %lf in %.12lf seconds using %d threads\n", num_steps, pi, run_time,requested_threads);
	return pi;
}

