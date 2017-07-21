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

int main(int argc, const char **argv)
{
  Gnuplot g1("lines");
  Gnuplot g2("lines");
  Gnuplot g3("lines");
  Gnuplot g4("lines");
  Gnuplot g5("lines");

	char name[10];
	int i = 1;

	//structures for images
	Rect ROI_right(0, 0, 128, 94); //Note that in the database, image left and right is reversed!
	Rect ROI_left(128, 0, 128, 94);
	Mat image_left_gray;
	Mat image_right_gray;
	Mat image;
	uint8_t image_buffer[128 * 94 * 2];
	uint8_t image_buffer_left[128 * 94 ];
	uint8_t image_buffer_right[128 * 94 ];

	// open video capture
	VideoCapture cap("/home/kirk/mavlab/SmartUAV/MATLAB/DelFlyInvestigator/rooms_dataset/singapore_rooms/left%d.png");
	time_inc = 1/7.;

/*	VideoCapture cap("stereoboard_database/Track3/%d.bmp");
  output.open("stereoboard_database/Track3/result.csv");
  edgeflow_params.stereo_shift = 2;
  time_inc = 1/8.;*/

  Mat prev_image;
  char prev_image_file[255];

	if (!cap.isOpened()) return -1;

	//start loop while images are read
	int counter = 0;
	for(;;) {
		counter++;
		cap >> image;

		if (image.empty()) {
			break;
		}

		namedWindow( "image", WINDOW_AUTOSIZE );// Create a window for display.
    imshow( "image", image );                   // Show our image inside it.
    waitKey(1);

		// Crop out the seperate left and right images
		if (image.channels() == 3) {
			cvtColor(image(ROI_left), image_left_gray, COLOR_BGR2GRAY);
			cvtColor(image(ROI_right), image_right_gray, COLOR_BGR2GRAY);
		} else {
			image_left_gray = image(ROI_left);
			image_right_gray = image(ROI_right);
		}

		// Put image values in array, just like in the stereoboard
		int x, y, idx,idx2;
		for (y = 0; y < image_left_gray.rows; y++) {
			for (x = 0; x < image_left_gray.cols; x++)
			{
				idx2 = image_left_gray.cols * y + x;

				image_buffer_left[idx2] = (uint8_t)image_left_gray.at<uchar>(y, x);
				image_buffer_right[idx2] = (uint8_t)image_right_gray.at<uchar>(y, x);

#if (CAMERA_CPLD_STEREO == camera_cpld_stereo_pixmux)
        idx = 2 * (image_left_gray.cols * y + x);
        image_buffer[idx] = (uint8_t)image_left_gray.at<uchar>(y, x);
        image_buffer[idx + 1] = (uint8_t)image_right_gray.at<uchar>(y, x);
#elif (CAMERA_CPLD_STEREO == camera_cpld_stereo_linemux)
        idx = 2 * image_left_gray.cols * y + x;
        image_buffer[idx] = (uint8_t)image_left_gray.at<uchar>(y, x);
        image_buffer[idx + image_left_gray.cols] = (uint8_t)image_right_gray.at<uchar>(y, x);
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
