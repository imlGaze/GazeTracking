#pragma once
#include <opencv2/opencv.hpp>
#include "RealSenseAPI/RealSenseAPI.h"
#include "const.h"

using namespace Intel::RealSense::Face;
using std::vector;
using cv::Point;
using cv::Rect;
using cv::Mat;
using cv::Mat_;

class PupilFinder {
	Mat H = (Mat_<float>(3, 3) << 0.7351358246144999, -0.0319196931238943, 37.4838452084103,
		-0.008067602816914776, 0.7099678351704652, 74.76441033183021,
		-2.565364511844114e-05, -0.0001259361553461414, 1);
	cv::VideoWriter wirb = DEBUG ? cv::VideoWriter("irb.avi", CV_FOURCC_DEFAULT, 30, cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), true) : cv::VideoWriter();

	bool findPupil(Mat crop, Point &pupil);
	Rect getEyeCrop(Point center, Point lidLeft, Point lidRight);
	void emphasize(Mat img);

public:
	int thresh = 100;
	int erode_filter = 3;
	int dilate_filter = 3;
	int erode_itr = 2;
	int dilate_itr = 3;

	bool PupilFinder::find(Mat ir, Mat color, map<FaceData::LandmarkType, Point> landmarks, Point &leftPupil, Point &rightPupil);

};
