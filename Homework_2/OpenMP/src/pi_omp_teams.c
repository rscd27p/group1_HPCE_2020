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
uint32_t teams_number = 2;
uint32_t max_num_threads = 2;
double step;

// Internal functions declarations

double pi_opm_teams(uint32_t num_steps, uint32_t teams_number, uint32_t max_num_threads);

// Main code

int main(int argc, char **argv) {
	num_steps_teams = 100000000;
	teams_number = 8;
	max_num_threads = 8;
	pi_opm_teams(num_steps_teams, teams_number, max_num_threads);
	return 0;

}

double pi_opm_teams(uint32_t num_steps, uint32_t teams_number, uint32_t max_num_threads){
	uint32_t i;
	double x, pi, sum = 0.0;
	double start_time, run_time;

	step = 1.0 / (double)num_steps;

	start_time = omp_get_wtime();
	// Make for loop parallel with the teams construct
	#pragma omp teams distribute num_teams(teams_number) thread_limit(max_num_threads) reduction(+:sum) private (x)
	for (i = 1; i <= num_steps; i++) {
		x = (i - 0.5) * step;
		sum = sum + 4.0 / (1.0 + x * x);
	}
	pi = step * sum;
	run_time = omp_get_wtime() - start_time;
	printf("pi teams implementation with %d number of teams, %d steps is %lf in %.12lf seconds\n", teams_number, num_steps, pi, run_time);
	return pi;
}
