#pragma once
#include <opencv2/opencv.hpp>
#include "RealSenseAPI/RealSenseAPI.h"

using namespace cv;
using namespace Intel::RealSense::Face;
using std::vector;

class PupilFinder {
	int thresh = 80;
	int erode_filter = 3;
	int dilate_filter = 3;
	int erode_itr = 2;
	int dilate_itr = 3;

	Mat H = (Mat_<float>(3, 3) << 0.7351358246144999, -0.0319196931238943, 37.4838452084103,
		-0.008067602816914776, 0.7099678351704652, 74.76441033183021,
		-2.565364511844114e-05, -0.0001259361553461414, 1);

	bool findPupil(Mat crop, Point &pupil);
	Rect getEyeCrop(Point center, Point lidLeft, Point lidRight);
	void emphasize(Mat img);

public:
	void setThreshold(int thresh);
	void setErode(int erode_itr);
	void setDilate(int dilate_itr);

	bool PupilFinder::find(Mat ir, Mat color, map<FaceData::LandmarkType, Point> landmarks, Point &leftPupil, Point &rightPupil);

};

inline void PupilFinder::setThreshold(int thresh) {
	this->thresh = thresh;
}

inline void PupilFinder::setErode(int erode_itr) {
	this->erode_itr = erode_itr;
}
inline void PupilFinder::setDilate(int dilate_itr) {
	this->dilate_itr = dilate_itr;
}
