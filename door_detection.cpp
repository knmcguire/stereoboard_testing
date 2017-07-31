#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <iostream>
//Include header file of stereoboard code
#include "../../stereoboard/edgeflow.h"
#include "../../stereoboard/main_parameters.h"
#include "gnuplot_i.hpp"

#include "image.h"

using namespace std;
using namespace cv;

/*
const int8_t FOVX = 104;   // 60deg = 1.04 rad
const int8_t FOVY = 79;    // 45deg = 0.785 rad
*/

#define FOVX 1.001819   // 57.4deg = 1.001819 rad
#define FOVY 0.776672    // 44.5deg = 0.776672 rad

uint32_t frame_time = 0;
float time_inc = 0;

#include "../../stereoboard/gate_detection.h"

uint16_t bin_cnt[3] = {0};
uint16_t bin_cnt_snake[3] = {0};
struct point_t roi[2];

struct gate_t gate;

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

  Gnuplot g1("lines");
  Gnuplot g2("lines");
  Gnuplot g3("lines");
  Gnuplot g4("lines");
  Gnuplot g5("lines");

	char name[10];
	int i = 1;

	//structures for images
	Mat image_l;
	Mat image_r;
	Mat grad(96, 128, CV_8UC1, Scalar(0,0,0));
	Mat grad_big(384, 512, CV_8UC1, Scalar(0,0,0));
	uint8_t image_buffer[128 * 96 * 2] = {0};
	uint8_t image_buffer_left[128 * 96] = {0};
	uint8_t image_buffer_right[128 * 96] = {0};
	uint8_t image_grad[128 * 96] = {0};

	// open video capture
	VideoCapture cap_l("/home/kirk/mavlab/SmartUAV/MATLAB/DelFlyInvestigator/rooms_dataset/singapore_rooms/left%d.png");
	VideoCapture cap_r("/home/kirk/mavlab/SmartUAV/MATLAB/DelFlyInvestigator/rooms_dataset/singapore_rooms/right%d.png");
	time_inc = 1/7.;

/*	VideoCapture cap("stereoboard_database/Track3/%d.bmp");
  output.open("stereoboard_database/Track3/result.csv");
  edgeflow_params.stereo_shift = 2;
  time_inc = 1/8.;*/

  Mat prev_image;
  char prev_image_file[255];

	if (!cap_l.isOpened() || !cap_r.isOpened()) return -1;

	//start loop while images are read
	int counter = 0;
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

    resize(image_l, image_l, Size(), 0.5, 0.5);
    resize(image_r, image_r, Size(), 0.5, 0.5);

		namedWindow( "image", WINDOW_AUTOSIZE );// Create a window for display.
    imshow( "image", image_r );                   // Show our image inside it.
    //waitKey(1);

		// Put image values in array, just like in the stereoboard
		int x, y, idx,idx2;
		for (y = 0; y < image_l.rows; y++) {
			for (x = 0; x < image_l.cols; x++)
			{
				idx2 = image_l.cols * y + x;

				image_buffer_left[idx2] = (uint8_t)image_l.at<uchar>(y, x);
				image_buffer_right[idx2] = (uint8_t)image_r.at<uchar>(y, x);

#if (CAMERA_CPLD_STEREO == camera_cpld_stereo_pixmux)
        idx = 2 * (image_l.cols * y + x);
        image_buffer[idx] = (uint8_t)image_l.at<uchar>(y, x);
        image_buffer[idx + 1] = (uint8_t)image_r.at<uchar>(y, x);
#elif (CAMERA_CPLD_STEREO == camera_cpld_stereo_linemux)
        idx = 2 * image_l.cols * y + x;
        image_buffer[idx] = (uint8_t)image_l.at<uchar>(y, x);
        image_buffer[idx + image_l.cols] = (uint8_t)image_r.at<uchar>(y, x);
#endif
			}
		}

		//dummyvalues
		int16_t *stereocam_data;
		uint8_t *edgeflowArray;

		frame_time += (uint32_t)(time_inc * 1e6);

		//calculate edgeflow
		std::vector<double> X, Y;
		std::vector<double> Y1, Y2, Y3, Y4, Y5, Y6, Y7, Y8, Y9, prev_hist, curr_hist, right_hist, dist_hist;

    printf("frame: %d\n", counter);

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

    printf("gate %d (%d,%d) %d %d %d\n", gate.q, gate.x, gate.y, gate.sz, gate.n_sides, gate.rot);

    for (int y = 0; y < image_l.rows; y++) {
      for (int x = 0; x < image_l.cols; x++) {
        grad.at<uchar>(y, x) = ((uint8_t*)d.buf)[x + y*image_l.cols];
      }
    }

    resize(grad, grad_big, Size(), 4, 4);
    namedWindow( "image", WINDOW_AUTOSIZE );      // Create a window for display.
    imshow( "after", grad_big );                   // Show our image inside it.
    waitKey(1);

		for(x=0; x < 128; x++)
		{
			X.push_back((double)x);

			//Y7.push_back((double)edgeflow.velocity_per_column[x]*edgeflow.disp.stereo[x]*100.);
			//Y2.push_back((double)(edgeflow.vel.x*100 + edgeflow.vel.z * (x-64)));
			//Y1.push_back((double)edgeflow.edge_hist_snapshot.x[x]);
			//Y2.push_back((double)(edgeflow_snapshot.dist.x*100 + edgeflow_snapshot.dist.z * (x-64)));

			//Y5.push_back((double)edgeflow.disp.x[x]);

			//Y3.push_back((double)edgeflow.disp.confidence_x[x]);
			//Y4.push_back((double)edgeflow.disp.confidence_stereo[x]);


			//prev_hist.push_back((double)edgeflow.edge_hist[edgeflow.prev_frame_x].x[x]);
			//prev_hist.push_back((double)edgeflow_snapshot.keyframe.x[x]);
			//curr_hist.push_back((double)edgeflow.edge_hist[(edgeflow.current_frame_nr - 1 + MAX_HORIZON) % MAX_HORIZON].x[x]);
			//right_hist.push_back((double)edgeflow.edge_hist_right[x]);

			//dist_hist.push_back((double)edgeflow.disp.stereo[x]);
		}

		for(y=0; y < 96; y++)
    {
		  //Y.push_back((double)y);
		  //Y6.push_back((double)edgeflow.disp.confidence_y[y]);
		  //Y7.push_back((double)edgeflow.disp.y[y]);
		  //Y7.push_back((double)edgeflow.vel.y[y]);
		  //Y8.push_back((double)(edgeflow.vel.y*100 + edgeflow.vel.z * (y-48)));

		  //Y8.push_back((double)(edgeflow_snapshot.dist.y*100 + edgeflow_snapshot.dist.z * (y-48)));
      //Y9.push_back((double)edgeflow.hist_match_quality_y[y]);
    }

		/*g1.set_grid();
		g2.set_grid();
		g3.set_grid();
		g4.set_grid();
		g5.set_grid();

		g1.plot_xy(X, Y1, "velocity_per_column");
		g1.plot_xy(X, Y2, "weighted flow fit");
		//g1.plot_xy(X, Y5, "standard flow fit");

		g2.plot_xy(X, Y3, "confidence x");
		g2.plot_xy(X, Y4, "confidence stereo");
		g2.plot_xy(Y, Y6, "confidence y");

		//g3.plot_xy(Y, Y7, "Y pixel displacement per column");
    //g3.plot_xy(Y, Y8, "Flow fit");

		g3.plot_xy(X, prev_hist, "prev_hist");
		g3.plot_xy(X, curr_hist, "curr_hist");

		g4.plot_xy(X, Y5, "left hist");
		//g5.plot_xy(X, right_hist, "right hist");

		g5.plot_xy(X, dist_hist, "disparity histogram");
		getchar();

		g1.reset_plot();
		g2.reset_plot();
		g3.reset_plot();
		g4.reset_plot();
		g5.reset_plot();*/

		std::cin.get();
	}

	return 0;
}
