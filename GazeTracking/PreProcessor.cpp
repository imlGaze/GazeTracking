#include "PreProcessor.h"

using namespace cv;

bool makeBinary(Mat gray, Mat &binary, int thresh) {
	Mat buffer(gray.size(), CV_8UC1);
	cvtColor(gray, buffer, CV_BGR2GRAY);

	buffer = ~buffer; // Invert Black and white
	threshold(buffer, binary, thresh, 255, CV_THRESH_BINARY);

	return true;
}

bool makeGray(Mat color, Mat &gray) {
	Mat buffer(color.size(), CV_8UC1);
	cvtColor(color, buffer, CV_BGR2GRAY);
	cvtColor(buffer, gray, CV_GRAY2BGR);

	return true;
}

bool emphasize(Mat src, Mat &dest) {
	dest = src.clone();

	Mat element(3, 3, CV_8UC1); // フィルタサイズ
	erode(dest, dest, element, Point(-1, -1), 2); // 収縮(ノイズ除去)、対象ピクセルの近傍のうち最大
	dilate(dest, dest, element, Point(-1, -1), 3); // 膨張（強調）、対象ピクセルの近傍のうち最小

	return true;
}
