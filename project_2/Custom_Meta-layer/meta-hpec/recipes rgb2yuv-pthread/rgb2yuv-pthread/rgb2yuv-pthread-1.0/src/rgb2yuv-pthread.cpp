/** @mainpage rgb2yuv_pthread - None
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
#include <pthread.h>


#define CLIP_uint16(X) ( (X) > 255 ? 255 : X)
#define CLIP_int16(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)

// Internal functions declaration below
void check_required_inputs(int i_flag, int o_flag, char *prog_name);
void main_run(char * rgb_image_path, char * yuv_image_path);
size_t load_input_file(char * file_name, unsigned char ** input_image);
void rgb2yuv(unsigned char *input_image, unsigned char *output_image, uint32_t total_bytes);
void* rgb2yuv_async_thread(void * data_ptr);
void rgb2yuv_main_thread(unsigned char *input_image, unsigned char *output_image, uint32_t bytes_to_process);

struct pthread_data{
	uint32_t bytes_to_process;
	unsigned char * input_image;
	unsigned char * output_image;
};

// Constant for help on usage
static char usage[] = "usage: %s [-h] [-a] -i RGBfile -o YUVfile\n"
		"-i RGBfile specifies the RGB file to be converted.\n"
		"-o YUVfile specifies the output file name.\n"
		"-a displays the information of the authors of the program.\n"
		"-h displays the usage message to let the user know how to execute the application.\n";

int main(int argc, char **argv) {
	// Use flags below to tell if the required arguments were provided
	int i_flag = 0;
	int o_flag = 0;

	// Variables below will save the path of the files provided by the user
	char rgb_image_path[128];
	char yuv_image_path[128];
	// Parse command line parameters
	int c;
	while ((c = getopt(argc, argv, "hai:o:")) != -1)
	switch (c){
		case 'i':
			i_flag = 1;
			snprintf(rgb_image_path, 128, "%s", optarg);
			break;
		case 'o':
			o_flag = 1;
			snprintf(yuv_image_path, 128, "%s", optarg);
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
	check_required_inputs(i_flag, o_flag, argv[0]);

	// Now run required functionality
	main_run(rgb_image_path, yuv_image_path);

	return 0;
}

// Function to check user provided the required arguments
void check_required_inputs(int i_flag, int o_flag, char *prog_name){
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
}

// Core component of main. Loads input file, does conversion and saves results
void main_run(char * rgb_image_path, char * yuv_image_path){
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
	rgb2yuv(input_image, output_image, bytes_read);
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
void rgb2yuv(unsigned char *input_image, unsigned char *output_image, uint32_t total_bytes){
	// Since we will have 4 workers, each of them has 1/4 bytes yo process
	uint32_t bytes_per_thread = total_bytes >> 2;
	pthread_t thread1, thread2, thread3;

	// Launch first thread
	struct pthread_data pt1_data;
	pt1_data.bytes_to_process = bytes_per_thread;
	pt1_data.input_image = input_image;
	pt1_data.output_image = output_image;
	pthread_create( &thread1, NULL, rgb2yuv_async_thread, (void*) &pt1_data);

	// Launch second thread with offset on data to process
	struct pthread_data pt2_data;
	pt2_data.bytes_to_process = bytes_per_thread;
	pt2_data.input_image = pt1_data.input_image + bytes_per_thread;
	pt2_data.output_image = pt1_data.output_image + bytes_per_thread;
	pthread_create( &thread2, NULL, rgb2yuv_async_thread, (void*) &pt2_data);

	// Launch third thread with offset on data to process
	struct pthread_data pt3_data;
	pt3_data.bytes_to_process = bytes_per_thread;
	pt3_data.input_image = pt2_data.input_image + bytes_per_thread;
	pt3_data.output_image = pt2_data.output_image + bytes_per_thread;
	pthread_create( &thread3, NULL, rgb2yuv_async_thread, (void*) &pt3_data);

	// Fourth thread is actually current thread
	rgb2yuv_main_thread(pt3_data.input_image + bytes_per_thread, pt3_data.output_image + bytes_per_thread, bytes_per_thread);

	// Wait for async threads to return
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
}

// Separate thread worker for rgb to yuv conversion
void* rgb2yuv_async_thread(void * data_ptr){
	// Convert input generic ptr to our pthread data pointer
	struct pthread_data * pt_data = (struct pthread_data *)data_ptr;

	// Prepare variables
	uint32_t bytes_counter = 0;
	uint8_t R, G, B;
	uint16_t Y_tmp;
	int16_t U_tmp, V_tmp;
	// Run through bytes in RGB image. They are increased by 3 every iteration
	for(bytes_counter=0; bytes_counter < pt_data->bytes_to_process; bytes_counter += 3){
		// Extract R, G and B
		R = pt_data->input_image[bytes_counter];
		G = pt_data->input_image[bytes_counter + 1];
		B = pt_data->input_image[bytes_counter + 2];
		// Calculate Y
		Y_tmp = ((66*R + 129*G + 25*B) + 128) >> 8;
		pt_data->output_image[bytes_counter] = (uint8_t) CLIP_uint16(Y_tmp + 16);
		// Calculate U
		U_tmp = ((-38*R - 74*G + 112*B) + 128) >> 8;
		pt_data->output_image[bytes_counter + 1] = (uint8_t) CLIP_int16(U_tmp + 128);
		// Calculate V
		V_tmp = ((112*R - 94*G - 18*B) + 128) >> 8;
		pt_data->output_image[bytes_counter + 2] = (uint8_t) CLIP_int16(V_tmp + 128);
	}
	return NULL;
}

// Worker to still call from main thread
void rgb2yuv_main_thread(unsigned char *input_image, unsigned char *output_image, uint32_t bytes_to_process){
	// Prepare variables
	uint32_t bytes_counter = 0;
	uint8_t R, G, B;
	uint16_t Y_tmp;
	int16_t U_tmp, V_tmp;
	// Run through bytes in RGB image. They are increased by 3 every iteration
	for(bytes_counter=0; bytes_counter<bytes_to_process; bytes_counter += 3){
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
