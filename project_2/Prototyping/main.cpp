/** @mainpage rgb_yuv_opencv - None
 *
 * @version 1.0.0
**/


#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;

int main(int argc, char **argv) {
	Mat rgb_image;
	Mat yuv_image;

	printf("Loading original RGB image ...\n");
	rgb_image = imread("imagejpg.jpg", IMREAD_UNCHANGED);
	// Confirm image was loaded correctly
	if(!rgb_image.data )
	{
		printf("There was a problem loading the RBG image. Aborting!\n");
		return -1;
	}
	int img_height = rgb_image.size[0];
	int img_width = rgb_image.size[1];
	printf("Image loaded. Height %d. Width %d\n", img_height, img_width);

	printf("Converting image to YUV ..\n");
	yuv_image = rgb_image.clone();
	cvtColor(rgb_image, yuv_image, COLOR_BGR2YUV);

	printf("Saving converted image to new file ...\n");
	FILE * output_file = fopen("imageyuv.yuv", "wb");
	size_t bytes_written = fwrite(yuv_image.data, 1, 3*img_height*img_width, output_file);
	printf("File imageyuv.yuv written with %ld bytes\n", bytes_written);

	return 0;
}
