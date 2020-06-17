/*
 ============================================================================
 Name        : trapezoidal.c
 Author      : agomez and rcespedes
 Version     : 1.0.0
 Description : General purpose: Composite Trapezoidal Rule Application
 Course: MP-6171 High Performance Embedded Systems
 Tecnologico de Costa Rica (www.tec.ac.cr)
 Input: Lower value, upper value and sub-intervals
 Output: numerical approximation
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Functions declaration
void check_required_inputs(int l_flag, int u_flag, int n_flag, char *prog_name);
void check_inputs_values(float lower, float upper, int n);
float Trapezoidal(float lower, float upper, int subInterval);
float f(float x);
void testing(void);

// Constant for help on usage
static char usage[] = "usage: %s [-h] [-a] -l LowerValue -u UpperValue -n subIntervalsValue\n"
		"-l LowerValue specifies lower value.\n"
		"-u UpperValue specifies upper value.\n"
		"-n subIntervalsValue specifies the sub-intervals value.\n"
		"-a displays the information of the authors of the program.\n"
		"-h displays the usage message to let the user know how to execute the application.\n";


int main(int argc, char *argv[]) {
	// Enable line below for testing
	//testing();

	// Define required parameters for calculation
	float lower;
	float upper;
	int n;

	// Use flags below to tell if the required arguments were provided
	int l_flag = 0;
	int u_flag = 0;
	int n_flag = 0;

	// Parse command line parameters
	int c;
	while ((c = getopt (argc, argv, "hal:u:n:")) != -1)
	switch (c){
		case 'l':
			l_flag = 1;
			lower = atof(optarg);
			break;
		case 'u':
			u_flag = 1;
			upper = atof(optarg);
			break;
		case 'n':
			n_flag = 1;
			n = atoi(optarg);
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
	// Check that required inputs were provided
	check_required_inputs(l_flag, u_flag, n_flag, argv[0]);
	// Verify inputs have values that make sense
	check_inputs_values(lower, upper, n);

	// Now we are ready to do calculation
	puts("Starting calculation...");
	float result = Trapezoidal(lower, upper, n);
	printf("Integral Result is %.8f\n", result);

	return EXIT_SUCCESS;
}


void check_required_inputs(int l_flag, int u_flag, int n_flag, char *prog_name){
	// Check required arguments were provided. Print error and abort otherwise
	if (!l_flag){
		fprintf(stderr, "%s: missing -l option\n", prog_name);
		fprintf(stderr, usage, prog_name);
		exit(1);
	}
	if (!u_flag){
		fprintf(stderr, "%s: missing -u option\n", prog_name);
		fprintf(stderr, usage, prog_name);
		exit(1);
	}
	if (!n_flag){
		fprintf(stderr, "%s: missing -n option\n", prog_name);
		fprintf(stderr, usage, prog_name);
		exit(1);
	}
}

// This function checks that the input values meet certain conditions
void check_inputs_values(float lower, float upper, int n){
	// Confirm upper is greater than lower
	if(lower >= upper){
		printf("ERROR: UpperValue must be a number greater than LowerValue\n");
		exit(1);
	}
	// Confirm n is greater than 0
	if(n <= 0){
		printf("ERROR: subIntervalsValue must be a positive integer number\n");
		exit(1);
	}
}

// This function calculates the integral of a function using the composite trapezoidal rule description
float Trapezoidal(float lower, float upper, int subInterval){
	// Start by calculating h
	float h = (upper - lower) / subInterval;

	// Calculate initial value for result
	float integral = f(lower) + f(upper);

	// Now add the iterative portion
	float sum = 0;
	for(int j=1; j<=(subInterval-1); j++){
		sum = sum + f(lower + (j*h));
	}

	// Calculate final value for integral result
	integral = h*(integral + 2*sum)/2;

	return integral;
}

// This function calculates the value of f(x) = y = 1 / (1+ x2)
float f(float x){
	float y = 1 / (1 + x*x);
	return y;
}

// Function used for testing purposes only
void testing(void){
	// Define constant values for test case 1
	float a = 0;
	float b = 10;
	int n = 200;
	float expected_result = 1.47112726584903;
	printf("Test 1 >> Integral Result: %.10f. Expected Result: %.10f\n\n", Trapezoidal(a, b, n), expected_result);

	// Define constant values for test case 2
	a = 3.5;
	b = 45;
	n = 150;
	expected_result = 0.256335035737283;
	printf("Test 2 >> Integral Result: %.10f. Expected Result: %.10f\n\n", Trapezoidal(a, b, n), expected_result);
}
