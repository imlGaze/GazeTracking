#pragma once

#include <opencv2\opencv.hpp>

using namespace cv;

bool makeBinary(Mat gray, Mat &binary, int thresh = 100);

bool makeGray(Mat color, Mat &gray);

bool emphasize(Mat src, Mat &dest);


