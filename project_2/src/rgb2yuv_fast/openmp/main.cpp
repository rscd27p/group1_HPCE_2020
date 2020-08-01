/** @mainpage rgb2yuv_fast_openmp - None
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
#include <arm_neon.h>
#include <omp.h>


#define CLIP_uint16(X) ( (X) > 255 ? 255 : X)
#define CLIP_int16(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)

// Internal functions declaration below
void check_required_inputs(int i_flag, int o_flag, char *prog_name);
void main_run(char * rgb_image_path, char * yuv_image_path);
size_t load_input_file(char * file_name, unsigned char ** input_image);
void rgb2yuv(unsigned char *input_image, unsigned char *output_image, uint32_t total_bytes);

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
void rgb2yuv(unsigned char * __restrict input_image, unsigned char * __restrict output_image, uint32_t total_bytes){
	// Temporal registers for calculations. Each of them has 8, 16-bit registers
	uint16x8_t  tmp_high_u;
	uint16x8_t  tmp_low_u;
	int16x8_t  tmp_high_s;
	int16x8_t  tmp_low_s;

	// Offset constant
	//int16x8_t  offset_128_i = vdupq_n_s16(128);

	// In each iteration of the for loop below we will process 16 pixels (48 bytes)
	uint32_t iterations = total_bytes / 48;
	uint32_t bytes_count = 0;

	// Triple register to store yuv results. Each register has 16, 8-bits registers
	uint8x16x3_t yuv;

	omp_set_num_threads(4);
	#pragma omp parallel for firstprivate(bytes_count), private(tmp_high_u, tmp_low_u, tmp_high_s, tmp_low_s, yuv)
	// Run through calculated number of iterations
	for(uint32_t it_count=0; it_count < iterations; it_count++){
		int16x8_t  offset_128_i = vdupq_n_s16(128);
		// Extract R, G and B onto triple register. Each register has 16, 8-bits registers
		uint8x16x3_t rgb  = vld3q_u8(input_image + bytes_count);

		// We are going to process each color in two chunks.
		uint8x8_t R_high_u = vget_high_u8(rgb.val[0]);
		uint8x8_t R_low_u = vget_low_u8(rgb.val[0]);
		uint8x8_t G_high_u = vget_high_u8(rgb.val[1]);
		uint8x8_t G_low_u = vget_low_u8(rgb.val[1]);
		uint8x8_t B_high_u = vget_high_u8(rgb.val[2]);
		uint8x8_t B_low_u = vget_low_u8(rgb.val[2]);

		// ******  Calculating Y ********
		//Y_tmp = ((66*R + 129*G + 25*B) + 128) >> 8;
		//output_image[bytes_counter] = (uint8_t) CLIP_uint16(Y_tmp + 16);
		// Perform multiplications by coefficients and accumulate for each of the 2 chunks
		tmp_high_u = vmull_u8(R_high_u, vdup_n_u8(66));
		tmp_low_u = vmull_u8(R_low_u, vdup_n_u8(66));

		tmp_high_u = vmlal_u8(tmp_high_u, G_high_u, vdup_n_u8(129));
		tmp_low_u = vmlal_u8(tmp_low_u, G_low_u, vdup_n_u8(129));

		tmp_high_u = vmlal_u8(tmp_high_u, B_high_u, vdup_n_u8(25));
		tmp_low_u = vmlal_u8(tmp_low_u, B_low_u, vdup_n_u8(25));

		// Add 128 constant
		tmp_high_u = vaddq_u16(tmp_high_u, vdupq_n_u16(128));
		tmp_low_u = vaddq_u16(tmp_low_u, vdupq_n_u16(128));

		// Now shift 8 bits to the right
		tmp_high_u = vshrq_n_u16(tmp_high_u, 8);
		tmp_low_u = vshrq_n_u16(tmp_low_u, 8);

		// Add 16 constant
		tmp_high_u = vaddq_u16(tmp_high_u, vdupq_n_u16(16));
		tmp_low_u = vaddq_u16(tmp_low_u, vdupq_n_u16(16));

		// Now saturate and convert back to 16, 8-bits registers which is final result for Y
		yuv.val[0] = vcombine_u8(vqmovn_u16(tmp_low_u), vqmovn_u16(tmp_high_u));

		// For U and V we need new values for RGB that are signed. In the same way, process in 2 chunks.
		int16x8_t R_high_s = vreinterpretq_s16_u16(vaddl_u8(R_high_u, vdup_n_u8(0)));
		int16x8_t R_low_s = vreinterpretq_s16_u16(vaddl_u8(R_low_u, vdup_n_u8(0)));
		int16x8_t G_high_s = vreinterpretq_s16_u16(vaddl_u8(G_high_u, vdup_n_u8(0)));
		int16x8_t G_low_s = vreinterpretq_s16_u16(vaddl_u8(G_low_u, vdup_n_u8(0)));
		int16x8_t B_high_s = vreinterpretq_s16_u16(vaddl_u8(B_high_u, vdup_n_u8(0)));
		int16x8_t B_low_s = vreinterpretq_s16_u16(vaddl_u8(B_low_u, vdup_n_u8(0)));

		// ******  Calculating U ********
		//U_tmp = ((-38*R - 74*G + 112*B) + 128) >> 8;
		//output_image[bytes_counter + 1] = (uint8_t) CLIP_int16(U_tmp + 128);
		// Perform multiplications by coefficients and accumulate for each of the 2 chunks
		tmp_high_s = vmulq_s16(R_high_s, vdupq_n_s16(-38));
		tmp_low_s = vmulq_s16(R_low_s, vdupq_n_s16(-38));

		tmp_high_s = vmlaq_s16(tmp_high_s, G_high_s, vdupq_n_s16(-74));
		tmp_low_s = vmlaq_s16(tmp_low_s, G_low_s, vdupq_n_s16(-74));

		tmp_high_s = vmlaq_s16(tmp_high_s, B_high_s, vdupq_n_s16(112));
		tmp_low_s = vmlaq_s16(tmp_low_s, B_low_s, vdupq_n_s16(112));

		// Add 128 constant
		tmp_high_s = vaddq_s16(tmp_high_s, offset_128_i);
		tmp_low_s = vaddq_s16(tmp_low_s, offset_128_i);

		// Now shift 8 bits to the right
		tmp_high_s = vshrq_n_s16(tmp_high_s, 8);
		tmp_low_s = vshrq_n_s16(tmp_low_s, 8);

		// Add 128 constant
		tmp_high_s = vaddq_s16(tmp_high_s, offset_128_i);
		tmp_low_s = vaddq_s16(tmp_low_s, offset_128_i);

		// Now saturate and convert back to 16, 8-bits registers which is final result for U
		yuv.val[1] = vcombine_u8(vqmovun_s16(tmp_low_s), vqmovun_s16(tmp_high_s));


		// ******  Calculating V ********
		//V_tmp = ((112*R - 94*G - 18*B) + 128) >> 8;
		//output_image[bytes_counter + 2] = (uint8_t) CLIP_int16(V_tmp + 128);
		// Perform multiplications by coefficients and accumulate for each of the 2 chunks
		tmp_high_s = vmulq_s16(R_high_s, vdupq_n_s16(112));
		tmp_low_s = vmulq_s16(R_low_s, vdupq_n_s16(112));

		tmp_high_s = vmlaq_s16(tmp_high_s, G_high_s, vdupq_n_s16(-94));
		tmp_low_s = vmlaq_s16(tmp_low_s, G_low_s, vdupq_n_s16(-94));

		tmp_high_s = vmlaq_s16(tmp_high_s, B_high_s, vdupq_n_s16(-18));
		tmp_low_s = vmlaq_s16(tmp_low_s, B_low_s, vdupq_n_s16(-18));

		// Add 128 constant
		tmp_high_s = vaddq_s16(tmp_high_s, offset_128_i);
		tmp_low_s = vaddq_s16(tmp_low_s, offset_128_i);

		// Now shift 8 bits to the right
		tmp_high_s = vshrq_n_s16(tmp_high_s, 8);
		tmp_low_s = vshrq_n_s16(tmp_low_s, 8);

		// Add 128 constant
		tmp_high_s = vaddq_s16(tmp_high_s, offset_128_i);
		tmp_low_s = vaddq_s16(tmp_low_s, offset_128_i);

		// Now saturate and convert back to 16, 8-bits registers which is final result for V
		yuv.val[2] = vcombine_u8(vqmovun_s16(tmp_low_s), vqmovun_s16(tmp_high_s));

		// Store yuv results interleaved to output image
		vst3q_u8(output_image + bytes_count, yuv);

		// Increase bytes counter with progress
		bytes_count = (it_count + 1) * 48;
	}
}



