#pragma once

#include "RealSenseAPI/RealSenseAPI.h"
#include <opencv2\opencv.hpp>

using namespace cv;
using std::map;
using Intel::RealSense::Face::FaceData;


bool makeBinary(Mat gray, Mat &binary, int thresh = 100);

bool makeGray(Mat color, Mat &gray);

bool emphasize(Mat src, Mat &dest);



