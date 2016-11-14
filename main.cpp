#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <iostream>
//Include header file of stereoboard code
#include "../stereoboard/edgeflow.h"
#include "gnuplot_i.hpp"

using namespace std;
using namespace cv;


/*
const int8_t FOVX = 104;   // 60deg = 1.04 rad
const int8_t FOVY = 79;    // 45deg = 0.785 rad
 */

#define FOVX 104
#define FOVY 79




void plot_edge_histogram(int32_t *edge_histogram);

int main(int argc, const char **argv)
{

	// initialize structures for plotting
/*	Gnuplot g1("lines");
	Gnuplot g2("lines");
	Gnuplot g3;
	Gnuplot g4;*/

	//parameters for edgeflow
	struct edgeflow_parameters_t edgeflow_parameters;
	struct edgeflow_results_t edgeflow_results;
	int32_t edge_histogram_x[IMAGE_WIDTH];
	//initialize for edgeflow
	edgeflow_init(&edgeflow_parameters, &edgeflow_results, FOVX, FOVY, 128, 96, 0);

	char name[10];
	int i = 1;

	//structures for images
	Rect ROI_right(0, 0, 128, 94); //Note that in the database, image left and right is reverted!
	Rect ROI_left(128, 0, 128, 94);
	Mat image_left, image_left_gray;
	Mat image_right, image_right_gray;
	Mat image;
	uint8_t image_buffer[128 * 96 * 2];
	uint8_t image_buffer_left[128 * 96 ];
	uint8_t image_buffer_right[128 * 96 ];

	memset(image_buffer, 0, 128 * 96 * 2 * sizeof(uint8_t));

	// open cvs file
	ofstream output;
	// open video capture
	VideoCapture cap;
	cap.open("stereoboard_database/Take16/%1d.bmp");
	output.open("stereoboard_database/Take16/result.csv");
	edgeflow_parameters.stereo_shift = -5; // calibration data of that file

	/*  cap.open("stereoboard_database/Track3/%1d.bmp");
  output.open("stereoboard_database/Track3/result.csv");
  edgeflow_parameters.stereo_shift = 2;*/

	//start loop while images are read
	int counter = 0;
	while (cap.isOpened()) {

		counter++;
		cap.read(image);

		if (image.empty()) {
			break;
		}
		// Crop out the seperate left and right images
		if (image.channels() == 3) {
			image_left = image(ROI_left);
			image_right = image(ROI_right);
			cvtColor(image_left, image_left_gray, COLOR_BGR2GRAY);
			cvtColor(image_right, image_right_gray, COLOR_BGR2GRAY);
		} else {
			image_left_gray = image(ROI_left);
			image_right_gray = image(ROI_right);
		}
	   // namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
	    //imshow( "Display window", image_left_gray );
	    //waitKey(0);
	    // Put image values in array, just like in the stereoboard
		int x, y, idx,idx2;
		for (y = 0; y < image.rows; y++) {
			for (x = 0; x < image.cols; x++)
			{
				idx = 2 * (128 * y + x);
				idx2 =  (128 * y + x);

				//TODO: this should be the right order?
				image_buffer_left[idx2] = (uint8_t)image_left_gray.at<uchar>(y, x);
				image_buffer_right[idx2] = (uint8_t)image_right_gray.at<uchar>(y, x);
				image_buffer[idx] = (uint8_t)image_left_gray.at<uchar>(y, x);
				image_buffer[idx+1] = (uint8_t)image_right_gray.at<uchar>(y, x);
			}
		}

		//dummyvalues
		int16_t *stereocam_data;
		uint8_t *edgeflowArray;

		//calculate edgeflow
		edgeflow_total(edgeflowArray, stereocam_data, 0, image_buffer,
				&edgeflow_parameters, &edgeflow_results);
		std::vector<double> X ;
		/*std::vector<double> Y1, Y2, Y3, Y4;
		for(x=0;x<128;x++)
		{
			X.push_back((double)x);

			Y1.push_back((double)edgeflow_results.velocity_per_column[x]);
			Y2.push_back((double)(edgeflow_results.vel_x_pixelwise + edgeflow_results.vel_z_pixelwise * (x-64) ));
			Y3.push_back((double)edgeflow_results.stereo_distance_per_column[x]/100);
			Y4.push_back((double)edgeflow_results.displacement.x[x]);
		}

		g1.set_grid();
		g2.set_grid();

		g1.set_style("lines").plot_xy(X,Y1,"displacement flow");
		g1.set_style("lines").plot_xy(X,Y2,"displacement stereo");
		g2.set_style("lines").plot_xy(X,Y3,"velocity_per_column");
		g2.set_style("lines").plot_xy(X,Y4,"velocity_per_column");
		getchar();

		g1.reset_plot();
		g2.reset_plot();

		if( counter >40)//TODO: get rid of those irritating warnings...
		{
			g1.remove_tmpfiles();
			g2.remove_tmpfiles();
		}*/


		//Save data on output.cvs
		//TODO: also enter groundtruth data
		output << (int)edgeflow_results.vel_x_pixelwise << "," << (int)edgeflow_results.vel_z_pixelwise <<
				", " << (int)edgeflow_results.vel_x_global << "," << (int)edgeflow_results.vel_y_global <<
				"," << (int)edgeflow_results.vel_z_global << "," << (int)edgeflow_results.velocity_stereo_mean <<
				"," << (int)edgeflow_results.vel_x_stereo_avoid_pixelwise << "," << (int)edgeflow_results.vel_z_stereo_avoid_pixelwise
				<< ", " << (int)edgeflow_results.avg_dist << endl;
	}

	output.close();

	return 0;
}

// Edgeflow function to plot some variables in a window
//TODO: find a nice plotting library for this
void plot_edge_histogram(int32_t *edge_histogram)
{
	int size = 128;

	cv::Mat edge_histogram_plot = cv::Mat::zeros(128, size, CV_8UC3);
	cv::Point line1, line2;
	cv::Point line1_prev, line2_prev;
	cv::Point line1_disp, line2_disp;

	line1.x = 0;
	line1.y = 0;

	for (int x = 1; x < size; x++) {

		line2.x = x;
		line2.y = (int)edge_histogram[x] / 10 + 60;


		cv::line(edge_histogram_plot, line1, line2, Scalar(0, 255, 0));



		line1.x = line2.x;
		line1.y = line2.y;

		line1_prev.x = line2_prev.x;
		line1_prev.y = line2_prev.y;


	}


	flip(edge_histogram_plot, edge_histogram_plot, 0);
	cv::namedWindow("edge_histogram", CV_WINDOW_NORMAL);

	cv::imshow("edge_histogram", edge_histogram_plot);


}



