#pragma once

#include <opencv2\opencv.hpp>

using namespace cv;
using std::vector;

bool findPupils(Mat binary, vector<Rect> &pupils);

void RenderEye(Mat color, Mat ir, Mat bin, Mat h, Point eye, Point eyeLidL, Point eyeLidR);
