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
#include <stdlib.h>

// Declare variables

uint32_t num_steps_teams = 100000000;
uint32_t teams_number = 2;
uint32_t max_num_threads = 2;
double step;

// Constant for help on usage
static char usage[] = "usage: %s [-h] [-a] -t teams_number -n thread_number\n"
		"-t Maximum number of teams.\n"
		"-n Maximum number of threads.\n"
		"-a displays the information of the authors of the program.\n"
		"-h displays the usage message to let the user know how to execute the application.\n";

// Internal functions declarations

double pi_opm_teams(uint32_t num_steps, uint32_t teams_number, uint32_t max_num_threads);
void check_required_inputs(int i_flag, int o_flag, char *prog_name);

// Main code

int main(int argc, char **argv) {
	// Use flags below to tell if the required arguments were provided
	int t_flag = 0;
	int n_flag = 0;
	
	// To save number of threads to use
	uint32_t max_num_threads;
	
	// To save number of teams to use
	uint32_t teams_number;
	
	int c;
	while ((c = getopt(argc, argv, "hat:n:")) != -1)
	switch (c){
		case 't':
			t_flag = 1;
			teams_number = atoi(optarg);
			break;
		case 'n':
			n_flag = 1;
			max_num_threads = atoi(optarg);
			break;
		case 'h':
			fprintf(stderr, usage, argv[0]);
			exit(1);
			break;
		case 'a':
			printf("Authors: agomez and rcespedes\n");
			exit(1);
			break;
		case ':':
			break;
		case '?':
			fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			return 1;
		default:
			abort();
	}
	// Check required arguments were provided
	check_required_inputs(t_flag, n_flag, argv[0]);
	// number of teams to run
	num_steps_teams = 100000000;
	printf("Configured to use -Teams number: %d | -Max thread number: %d\n",teams_number, max_num_threads);
	pi_opm_teams(num_steps_teams, 8, 8);
	return 0;

}

// Function to check user provided the required arguments
void check_required_inputs(int t_flag, int n_flag, char *prog_name){
	// Check required arguments were provided. Print error and abort otherwise
	if (!t_flag){
		fprintf(stderr, "%s: missing -t option\n", prog_name);
		fprintf(stderr, usage, prog_name);
		exit(1);
	}
	if (!n_flag){
		fprintf(stderr, "%s: missing -n option\n", prog_name);
		fprintf(stderr, usage, prog_name);
		exit(1);
	}
}

double pi_opm_teams(uint32_t num_steps, uint32_t teams_number, uint32_t max_num_threads){
	// Declare Internal Variables
	uint32_t i, used_teams, used_threads = 0;
	double x, pi, sum = 0.0;
	double start_time, run_time;
	// Calcualte step size
	step = 1.0 / (double)num_steps;
	// get start time
	start_time = omp_get_wtime();
	// Make for loop parallel with the teams construct and a thread_limit
	
	// Make for loop parallel with the teams construct
	// Set reduction pragma to optimize sum operation and make the x variable private between threads
	#pragma omp teams num_teams(teams_number) thread_limit(max_num_threads)
	used_teams = omp_get_num_teams();
	used_threads = omp_get_num_threads();
	// the reduction operation makes the variable private and causes the system to perform a reduction optimization at the end of the parallel region. Also, uses the distribute construct to evenly distribute the loop executions among threads
	#pragma omp distribute parallel for reduction(+:sum) private (x)
	for (i = 1; i <= num_steps; i++) {
		x = (i - 0.5) * step;
		sum = sum + 4.0 / (1.0 + x * x);
	}
	pi = step * sum;
	run_time = omp_get_wtime() - start_time;
	printf("Current settings -Teams used number: %d | -Threads used: %d \n",used_teams, used_threads);
	printf("pi teams implementation with %d steps is %lf in %.12lf seconds\n", num_steps, pi, run_time);
	return pi;

}
