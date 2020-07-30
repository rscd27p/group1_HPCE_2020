/** @mainpage rgb2yuv_intrinsics - None
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
	// Coefficients for Y calculation. Each register is 64 bytes (8 repeated coefficients, each 1 byte)
	uint8x8_t y_r_coeff = vdup_n_u8(66);
	uint8x8_t y_g_coeff = vdup_n_u8(129);
	uint8x8_t y_b_coeff = vdup_n_u8(25);

	// Coefficients for U calculation. Each register is 128 bytes (8 repeated coefficients, each 2 bytes)
	int16x8_t u_r_coeff = vdupq_n_s16(-38);
	int16x8_t u_g_coeff = vdupq_n_s16(-74);
	int16x8_t u_b_coeff = vdupq_n_s16(112);

	// Coefficients for V calculation. Each register is 128 bytes (8 repeated coefficients, each 2 bytes)
	int16x8_t v_r_coeff = vdupq_n_s16(112);
	int16x8_t v_g_coeff = vdupq_n_s16(-94);
	int16x8_t v_b_coeff = vdupq_n_s16(-18);


	// Temporal registers for calculations. Each of them has 8, 16-bit registers
	uint16x8_t  Y_tmp;
	int16x8_t  U_tmp;
	int16x8_t  V_tmp;


	// Offset constants
	uint16x8_t  offset_128_u = vdupq_n_u16(128);
	uint16x8_t  offset_16_u = vdupq_n_u16(16);
	int16x8_t  offset_128_i = vdupq_n_s16(128);
	int16x8_t  offset_16_i = vdupq_n_s16(16);

	uint32_t iterations = total_bytes / 24;
	uint32_t it_count = 0;
	uint32_t bytes_count = 0;

	// Triple register to store yuv results
	uint8x8x3_t yuv;

	// Run through calculated number of iterations
	for(it_count=0; it_count < iterations; it_count++){
		// Extract R, G and B. We are loading in one call 8 bytes into 3 separate registers
		uint8x8x3_t rgb  = vld3_u8(input_image + bytes_count);

		// ******  Calculating Y ********
		//Y_tmp = ((66*R + 129*G + 25*B) + 128) >> 8;
		//output_image[bytes_counter] = (uint8_t) CLIP_uint16(Y_tmp + 16);
		// Multiply rgb by coefficients to obtain Y_tmp. Multiply values 8 by 8 and accumulate result on Y_tmp
		Y_tmp = vmull_u8(rgb.val[0], y_r_coeff);
		Y_tmp = vmlal_u8(Y_tmp, rgb.val[1], y_g_coeff);
		Y_tmp = vmlal_u8(Y_tmp, rgb.val[2], y_b_coeff);
		// Add 128 constant to the 8, 16-bit registers
		Y_tmp = vaddq_u16(Y_tmp, offset_128_u);
		// Now shift 8 bits to the right
		Y_tmp = vshrq_n_u16(Y_tmp, 8);
		// Add 16 constant to the 8, 16-bit registers
		Y_tmp = vaddq_u16(Y_tmp, offset_16_u);
		// Now saturate and convert back to 8, 8-bits registers which is final result for Y
		yuv.val[0] = vqmovn_u16(Y_tmp);


		// ******  Calculating U ********
		//U_tmp = ((-38*R - 74*G + 112*B) + 128) >> 8;
		//output_image[bytes_counter + 1] = (uint8_t) CLIP_int16(U_tmp + 128);
		// Multiply rgb by coefficients to obtain U_tmp. Multiply values 8 by 8 and accumulate result on U_tmp
		U_tmp = vmulq_s16(vreinterpretq_s16_u16(vmovl_u8 (rgb.val[0])), u_r_coeff);
		U_tmp = vmlaq_s16(U_tmp, vreinterpretq_s16_u16(vmovl_u8 (rgb.val[1])), u_g_coeff);
		U_tmp = vmlaq_s16(U_tmp, vreinterpretq_s16_u16(vmovl_u8 (rgb.val[2])), u_b_coeff);
		// Add 128 constant to the 8, 16-bit registers
		U_tmp = vaddq_s16(U_tmp, offset_128_i);
		// Now shift 8 bits to the right
		U_tmp = vshrq_n_s16(U_tmp, 8);
		// Add 128 constant to the 8, 16-bit registers
		U_tmp = vaddq_s16(U_tmp, offset_128_i);
		// Now saturate and convert back to 8, 8-bits registers which is final result for U
		yuv.val[1] = vqmovun_s16(U_tmp);

		// ******  Calculating V ********
		//V_tmp = ((112*R - 94*G - 18*B) + 128) >> 8;
		//output_image[bytes_counter + 2] = (uint8_t) CLIP_int16(V_tmp + 128);
		// Multiply rgb by coefficients to obtain U_tmp. Multiply values 8 by 8 and accumulate result on U_tmp
		V_tmp = vmulq_s16(vreinterpretq_s16_u16(vmovl_u8 (rgb.val[0])), v_r_coeff);
		V_tmp = vmlaq_s16(V_tmp, vreinterpretq_s16_u16(vmovl_u8 (rgb.val[1])), v_g_coeff);
		V_tmp = vmlaq_s16(V_tmp, vreinterpretq_s16_u16(vmovl_u8 (rgb.val[2])), v_b_coeff);
		// Add 128 constant to the 8, 16-bit registers
		V_tmp = vaddq_s16(V_tmp, offset_128_i);
		// Now shift 8 bits to the right
		V_tmp = vshrq_n_s16(V_tmp, 8);
		// Add 128 constant to the 8, 16-bit registers
		V_tmp = vaddq_s16(V_tmp, offset_128_i);
		// Now saturate and convert back to 8, 8-bits registers which is final result for U
		yuv.val[2] = vqmovun_s16(V_tmp);


		// Store results interleaved to output image
		vst3_u8(output_image + bytes_count, yuv);

		// Increase bytes counter with progress
		bytes_count += 24;
	}
}

