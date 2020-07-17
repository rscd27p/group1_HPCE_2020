cmake_minimum_required (VERSION 2.8.1)

######## Project settings ########
PROJECT(rgb_yuv_opencv)
SET(LICENSE "TBD")

######## Build and include settings ########
include_directories(
	inc
	/opt/poky/3.0.3/sysroots/aarch64-poky-linux/usr/include/opencv4
)

link_directories(
	${LINK_DIRECTORIES}
)


file(GLOB SOURCES
	"src/*.cpp"
)

add_executable(
	rgb_yuv_opencv

	${SOURCES}
)

TARGET_LINK_LIBRARIES(
	rgb_yuv_opencv
	opencv_core
	opencv_imgcodecs
	opencv_imgproc
	opencv_highgui
)

######## Install targets ########
INSTALL(TARGETS rgb_yuv_opencv
	RUNTIME DESTINATION usr/bin
)
