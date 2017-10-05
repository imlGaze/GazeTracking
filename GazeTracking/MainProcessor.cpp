#include "MainProcessor.h"

#include <opencv2\opencv.hpp>

using namespace cv;
using std::vector;

bool findPupils(Mat binary, vector<Rect> &pupils) {
	vector<vector<Point>> contours;
	
	// 輪郭(Contour)抽出、RETR_EXTERNALで最も外側のみ、CHAIN_APPROX_NONEですべての輪郭点（輪郭を構成する点）を格納
	findContours(binary, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	for (int i = 0, n = contours.size(); i < n; i++)
	{
		Rect rect = boundingRect(contours[i]); // 点の集合に外接する傾いていない矩形を求める
		pupils.push_back(rect);
	}

	return pupils.size() != 0;
}
