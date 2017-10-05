#pragma once

#include <opencv2\opencv.hpp>

using namespace cv;
using std::vector;

bool findPupils(Mat binary, vector<Rect> &pupils);
