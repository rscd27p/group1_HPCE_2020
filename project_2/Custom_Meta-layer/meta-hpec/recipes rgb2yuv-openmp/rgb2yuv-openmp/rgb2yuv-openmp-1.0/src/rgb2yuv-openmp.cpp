/** @mainpage rgb2yuv_openmp - None
 *
 * @author  <anony@mo.us>
 * @version 1.0.0
**/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>


#define CLIP_uint16(X) ( (X) > 255 ? 255 : X)
#define CLIP_int16(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)

// Internal functions declaration below
void check_required_inputs(int i_flag, int o_flag, int t_flag, char *prog_name);
void main_run(char * rgb_image_path, char * yuv_image_path, uint16_t threads_number);
size_t load_input_file(char * file_name, unsigned char ** input_image);
void rgb2yuv(unsigned char *input_image, unsigned char *output_image, uint32_t total_bytes, uint16_t threads_number);

// Constant for help on usage
static char usage[] = "usage: %s [-h] [-a] -i RGBfile -o YUVfile -t threads_number\n"
		"-i RGBfile specifies the RGB file to be converted.\n"
		"-o YUVfile specifies the output file name.\n"
		"-t threads_number specifies the number of threads to use.\n"
		"-a displays the information of the authors of the program.\n"
		"-h displays the usage message to let the user know how to execute the application.\n";

int main(int argc, char **argv) {
	// Use flags below to tell if the required arguments were provided
	int i_flag = 0;
	int o_flag = 0;
	int t_flag = 0;

	// To save number of threads to use
	uint16_t threads_number;

	// Variables below will save the path of the files provided by the user
	char rgb_image_path[128];
	char yuv_image_path[128];
	// Parse command line parameters
	int c;
	while ((c = getopt(argc, argv, "hai:o:t:")) != -1)
	switch (c){
		case 'i':
			i_flag = 1;
			snprintf(rgb_image_path, 128, "%s", optarg);
			break;
		case 'o':
			o_flag = 1;
			snprintf(yuv_image_path, 128, "%s", optarg);
			break;
		case 't':
			t_flag = 1;
			threads_number = atoi(optarg);
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
	check_required_inputs(i_flag, o_flag, t_flag, argv[0]);

	// Now run required functionality
	main_run(rgb_image_path, yuv_image_path, threads_number);

	return 0;
}

// Function to check user provided the required arguments
void check_required_inputs(int i_flag, int o_flag, int t_flag, char *prog_name){
	// Check required arguments were provided. Print error and abort otherwise
	if (!i_flag){
		fprintf(stderr, "%s: missing -i option\n", prog_name);
		fprintf(stderr, usage, prog_name);
		exit(1);
	}
	if (!o_flag){
		fprintf(stderr, "%s: missing -o option\n", prog_name);
		fprintf(stderr, usage, prog_name);
		exit(1);
	}
	if (!t_flag){
		fprintf(stderr, "%s: missing -t option\n", prog_name);
		fprintf(stderr, usage, prog_name);
		exit(1);
	}
}

// Core component of main. Loads input file, does conversion and saves results
void main_run(char * rgb_image_path, char * yuv_image_path, uint16_t threads_number){
	// Declare variables for benchmark
	struct timeval t_wall_start;
	struct timeval t_wall_end;
	clock_t t_clock_start;
	clock_t t_clock_end;

	// Load input image from file into buffer
	printf(">> Loading binary RGB file '%s'\n", rgb_image_path);
	//char const *file_name = "image.rgb";
	unsigned char *input_image;
	uint32_t bytes_read = (uint32_t)load_input_file(rgb_image_path, &input_image);
	printf("Bytes read from file %d\n", bytes_read);

	// Allocate buffer for output image
	unsigned char *output_image = (unsigned char*)malloc((sizeof(unsigned char)*bytes_read));

	// Call function to convert from RGB to YUV
	printf("\n>> Converting RGB to YUV\n");
	// Capture times before running function
	gettimeofday(&t_wall_start, 0);
	t_clock_start = clock();
	// Do conversion
	rgb2yuv(input_image, output_image, bytes_read, threads_number);
	// Capture times after running and print results
	t_clock_end = clock();
	gettimeofday(&t_wall_end, 0);
	printf ("Clock ticks spent %ld (%f seconds).\n",t_clock_end-t_clock_start,((float)t_clock_end-t_clock_start)/CLOCKS_PER_SEC);
	long wall_elapsed = (t_wall_end.tv_sec-t_wall_start.tv_sec)*1000000 + t_wall_end.tv_usec-t_wall_start.tv_usec;
	printf("Wall Time Elapsed in %ld us\n", wall_elapsed);

	// Save YUV image to file
	printf("\n>> Saving converted image to new file\n");
	FILE * output_file = fopen(yuv_image_path, "wb");
	size_t bytes_written = fwrite(output_image, 1, bytes_read, output_file);
	printf("File '%s' written with %ld bytes\n", yuv_image_path, bytes_written);

	// Release allocated memory
	free(input_image);
	free(output_image);
	free(output_file);

}

// Function to load input image file into char* buffer
size_t load_input_file(char * file_name, unsigned char ** input_image){
	// Open file and determine its size
	FILE * input_file = fopen(file_name, "rb");
	fseek(input_file, 0L, SEEK_END);
	size_t input_bytes = ftell(input_file);
	rewind(input_file);

	// Save file contents into input_image buffer
	*input_image = (unsigned char*)malloc((sizeof(char)*input_bytes));
	size_t bytes_read = fread(*input_image, 1, input_bytes, input_file);
	return bytes_read;
}

// Function to convert RGB to YUV
void rgb2yuv(unsigned char *input_image, unsigned char *output_image, uint32_t total_bytes, uint16_t threads_number){
	// Prepare variables
	uint32_t bytes_counter = 0;
	uint8_t R, G, B;
	uint16_t Y_tmp;
	int16_t U_tmp, V_tmp;
	// Make for loop parallel with openmp pragma
	omp_set_num_threads(threads_number);
	#pragma omp parallel for private(R, G, B, Y_tmp, U_tmp, V_tmp)
	// Run through bytes in RGB image. They are increased by 3 every iteration
	for(bytes_counter=0; bytes_counter<total_bytes; bytes_counter += 3){
		// Extract R, G and B
		R = input_image[bytes_counter];
		G = input_image[bytes_counter + 1];
		B = input_image[bytes_counter + 2];
		// Calculate Y
		Y_tmp = ((66*R + 129*G + 25*B) + 128) >> 8;
		output_image[bytes_counter] = (uint8_t) CLIP_uint16(Y_tmp + 16);
		// Calculate U
		U_tmp = ((-38*R - 74*G + 112*B) + 128) >> 8;
		output_image[bytes_counter + 1] = (uint8_t) CLIP_int16(U_tmp + 128);
		// Calculate V
		V_tmp = ((112*R - 94*G - 18*B) + 128) >> 8;
		output_image[bytes_counter + 2] = (uint8_t) CLIP_int16(V_tmp + 128);
	}
}

