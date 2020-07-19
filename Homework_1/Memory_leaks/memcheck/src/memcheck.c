// Compile with: gcc -o memcheck memcheck.c -ldl

/*
 ============================================================================
 Name        : memcheck.c
 Author      : agomez and rcespedes
 Version     : 1.0.0
 Description : General purpose: 
 Course: MP-6171 High Performance Embedded Systems
 Tecnologico de Costa Rica (www.tec.ac.cr)
 Input: 
 Output: 
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/wait.h>
#include "config.h"

// Declare variables

char *p = NULL;

// Constant for help on usage
static char usage[] = "Usage: %s [-p ./PROGRAM] [-h][-a]\n"
		"-a displays the information of the author of the program.\n"
		"-h displays the usage message to let the user know how to execute the application.\n"
		"-p PROGRAM specifies the path to the program binary that will be analyzed.\n";

// Use flags below to tell if the required arguments were provided
int p_flag = 0;

int main(int argc, char** argv){
	int c;
	while ((c = getopt (argc, argv, "p:ha")) != -1)
	switch (c){
		case 'p':
			p_flag = 1;
			p = optarg;
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
			if (optopt == 'p')
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			return 1;
		default:
			abort();
	}

// Load the program to analyze with the LD_PRELOAD variable set to our custom libmemcheck.so

	// case4 path: /home/project2/Documents/HPEC/group1_HPCE_2020/Homework_1/Memory_leaks/case4

	if (p_flag = 1){
		char *const program_path = p;
		// Enable line below for troubleshooting		
		//printf ("program path = %s\n", p);
    		char *const args[] = {program_path,NULL};
    		char *const envs[] = {"LD_PRELOAD=../lib/libmemcheck.so",NULL};
    		execve(program_path,args,envs);
		printf ("Program succesfully executed\n");
	}
    	return 0;
}


