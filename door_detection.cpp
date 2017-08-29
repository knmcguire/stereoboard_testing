#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <iostream>
//Include header file of stereoboard code
#include "../../stereoboard/gate_detection.h"
#include "../../stereoboard/main_parameters.h"
#include "gnuplot_i.hpp"

#include "image.h"

using namespace std;
using namespace cv;

uint16_t bin_cnt[3] = {0};
uint16_t bin_cnt_snake[3] = {0};
struct point_t roi[2];

struct gate_t gate;

bool make_movie = true;
const string video_name = "./door_video.avi";

uint32_t positive = 0, negative = 0;
int main(int argc, const char **argv)
{
  roi[0].x = 0;// (IMAGE_WIDTH - cal_width) / 2;
  roi[0].y = 0;//(IMAGE_HEIGHT - cal_height) / 2;
  roi[1].x = IMAGE_WIDTH;//(IMAGE_WIDTH + cal_width) / 2;
  // if we only want to look at possible floor points that are more than 7 meters away
  // we can exclude some of the points in the bottom of the frame
  // namely 16 deg of the FOV resulting in only looking at the top 62 pixels
  roi[1].y = 62;//(IMAGE_HEIGHT+cal_height)/2;

  gate_set_intensity(50, 255); // nus meeting room

	//structures for images
	Mat image_l;
	Mat image_l_big;
	Mat image_r;
	Mat grad(96, 128, CV_8UC1, Scalar(0,0,0));
	Mat grad_big(384, 512, CV_8UC1, Scalar(0,0,0));
	Mat vid_frame(384, 1024, CV_8UC1, Scalar(0,0,0));
	uint8_t image_buffer[128 * 96 * 2] = {0};
	uint8_t image_buffer_left[128 * 96] = {0};
	uint8_t image_buffer_right[128 * 96] = {0};
	uint8_t image_grad[128 * 96] = {0};

	// open video capture
	VideoCapture cap_l("/home/kirk/mavlab/SmartUAV/MATLAB/DelFlyInvestigator/rooms_dataset/singapore_rooms/left%d.png");
	VideoCapture cap_r("/home/kirk/mavlab/SmartUAV/MATLAB/DelFlyInvestigator/rooms_dataset/singapore_rooms/right%d.png");

  Mat prev_image;
  char prev_image_file[255];

	if (!cap_l.isOpened() || !cap_r.isOpened()) return -1;

	int counter = 0;

	cap_l >> image_l;
  cap_r >> image_r;
  counter++;

  VideoWriter outputVideo;                                        // Open the output
  if(make_movie){
    int codec = CV_FOURCC('M', 'J', 'P', 'G');  // select desired codec (must be available at runtime)
    outputVideo.open(video_name, codec, 6., vid_frame.size(), false);

    // check if we succeeded
    if (!outputVideo.isOpened()) {
        cerr << "Could not open the output video file for write\n";
        return -1;
    }
  }

	//start loop while images are read
	for(;;) {
    counter++;
    cap_l >> image_l;
    cap_r >> image_r;

		if (image_l.empty() || image_r.empty()) {
			break;
		}

		// Crop out the separate left and right images
    if (image_l.channels() == 3) {
      cvtColor(image_l, image_l, COLOR_BGR2GRAY);
      cvtColor(image_r, image_r, COLOR_BGR2GRAY);
    }

    namedWindow( "image", WINDOW_AUTOSIZE );// Create a window for display.
    imshow( "image", image_r );                   // Show our image inside it.

    resize(image_l, image_l, Size(), 0.5, 0.5);
    resize(image_r, image_r, Size(), 0.5, 0.5);

		// Put image values in array, just like in the stereoboard
		int x, y, idx,idx2;

    for (y = 0; y < image_l.rows; y++) {
      for (x = 1; x < image_l.cols; x++)  //stored images have coloums shifted by one, first coloum is actually last coloum
      {
        idx2 = image_l.cols * y + x - 1;

        image_buffer_left[idx2] = (uint8_t)image_l.at<uchar>(y, x);
        image_buffer_right[idx2] = (uint8_t)image_r.at<uchar>(y, x);

#if (CAMERA_CPLD_STEREO == camera_cpld_stereo_pixmux)
        idx = 2 * (image_l.cols * y + x - 1);
        image_buffer[idx] = (uint8_t)image_l.at<uchar>(y, x);
        image_buffer[idx + 1] = (uint8_t)image_r.at<uchar>(y, x);
#elif (CAMERA_CPLD_STEREO == camera_cpld_stereo_linemux)
        idx = 2 * image_l.cols * y + x - 1;
        image_buffer[idx] = (uint8_t)image_l.at<uchar>(y, x);
        image_buffer[idx + image_l.cols] = (uint8_t)image_r.at<uchar>(y, x);
#endif
      }
    }

    // fill first colomn
    for (y = 0; y < image_l.rows; y++)
    {
      idx2 = image_l.cols * y + image_l.cols - 1;
      image_buffer_left[idx2] = (uint8_t)image_l.at<uchar>(y+1, 0);
      image_buffer_right[idx2] = (uint8_t)image_r.at<uchar>(y+1, 0);

#if (CAMERA_CPLD_STEREO == camera_cpld_stereo_pixmux)
      idx = 2 * (image_l.cols * y + x);
      image_buffer[idx] = (uint8_t)image_l.at<uchar>(y+1, 0);
      image_buffer[idx + 1] = (uint8_t)image_r.at<uchar>(y+1, 0);
#elif (CAMERA_CPLD_STEREO == camera_cpld_stereo_linemux)
      idx = 2 * image_l.cols * y + x;
      image_buffer[idx] = (uint8_t)image_l.at<uchar>(y+1, 0);
      image_buffer[idx + image_l.cols] = (uint8_t)image_r.at<uchar>(y+1, 0);
#endif
    }

    struct image_t input;
    input.buf = image_buffer_left;
    input.w = IMAGE_WIDTH;
    input.h = IMAGE_HEIGHT;
    input.buf_size = IMAGE_WIDTH*IMAGE_HEIGHT;
    input.type = IMAGE_GRAYSCALE;

    struct image_t d;
    d.buf = image_grad;
    d.w = IMAGE_WIDTH;
    d.h = IMAGE_HEIGHT;
    d.buf_size = IMAGE_WIDTH*IMAGE_HEIGHT;
    d.type = IMAGE_GRAYSCALE;

    image_2d_gradients(&input, &d);
    //image_2d_sobel(&input, &d);

    for (int y = 0; y < image_l.rows; y++) {
      for (int x = 0; x < image_l.cols; x++) {
        grad.at<uchar>(y, x) = ((uint8_t*)d.buf)[x + y*image_l.cols];
      }
    }

    resize(grad, grad_big, Size(), 4, 4);
    namedWindow( "image", WINDOW_AUTOSIZE );      // Create a window for display.
    imshow( "before", grad_big );                   // Show our image inside it.

    memset(bin_cnt_snake, 0, sizeof(bin_cnt));  // reset the counters
    snake_gate_detection(&d, &gate, false, bin_cnt_snake, NULL, NULL);

    struct point_t roi[2];
    roi[0].x = gate.x-gate.sz; roi[0].y = gate.y-gate.sz_left;
	  roi[1].x = gate.x+gate.sz; roi[1].y = gate.y+gate.sz_left;
    //uint32_t avg = image_roi_mean(&d, roi);
    //printf("avg = %d\n", avg);
    printf("gate %d (%d,%d) %d %d %d\n", gate.q, gate.x, gate.y, gate.sz, gate.n_sides, gate.rot);

    for (int y = 0; y < image_l.rows; y++) {
      for (int x = 0; x < image_l.cols; x++) {
        grad.at<uchar>(y, x) = ((uint8_t*)d.buf)[x + y*image_l.cols];
      }
    }

    resize(grad, grad_big, Size(), 4, 4);
    namedWindow( "image", WINDOW_AUTOSIZE );      // Create a window for display.
    imshow( "after", grad_big );                   // Show our image inside it.

    resize(image_l, image_l_big, Size(), 4, 4);
    for (int y = 0; y < grad_big.rows; y++) {
      for (int x = 0; x < grad_big.cols; x++) {
        vid_frame.at<uchar>(y, x) = image_l_big.at<uchar>(y,x);
        vid_frame.at<uchar>(y, x + grad_big.cols) = grad_big.at<uchar>(y,x);
      }
    }

    waitKey(1);

    if (make_movie){
      // encode the frame into the videofile stream
      outputVideo.write(vid_frame);
    }

    /*if (gate.q > 15){
      char key = std::cin.get();
      printf("%d\n", key);
      if (key == 49){
        positive++;
      } else if(key == 48){
        negative++;
      }
	  }*/
	}
	printf("%d positivly identified doors. %d negative\n", positive, negative);

	return 0;
}
