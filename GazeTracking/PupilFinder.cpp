#include <opencv2/opencv.hpp>
#include <RealSense/Face/FaceData.h>

#include "PupilFinder.h"
#include "const.h"

using namespace Intel::RealSense;
using namespace Intel::RealSense::Face;

using std::vector;
using cv::Point;
using cv::Rect;
using cv::Mat;

bool PupilFinder::findPupil(Mat crop, Point &pupil) {
	vector<vector<Point>> contours;
	// 輪郭(Contour)抽出、RETR_EXTERNALで最も外側のみ、CHAIN_APPROX_NONEですべての輪郭点（輪郭を構成する点）を格納
	findContours(crop, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

	Size size = crop.size();
	vector<Rect> pupils;

	for (int i = 0, n = contours.size(); i < n; i++)
	{
		Rect rect = boundingRect(contours[i]); // 点の集合に外接する傾いていない矩形を求める
		// cv::rectangle(crop, rect, cv::Scalar(0, 0, 255), 2);

		Point lt = Point(rect.x, rect.y);
		Point rt = Point(rect.x + rect.width, rect.y);
		Point lb = Point(rect.x, rect.y + rect.height);
		Point rb = Point(rect.x + rect.width, rect.y + rect.height);

		/*
		const int margin = 8; // 適当
		if (lt.x <= margin || lb.x <= margin)  continue;
		if (lt.y <= margin || rt.y <= margin)  continue;
		if (size.width - margin <= rt.x || size.width - margin <= rb.x)  continue;
		if (size.height - margin <= lb.y || size.height - margin <= rb.y) continue;
		//枠に被るやつを除外
		//if (rect.width > rect.height) continue; //横長を除外
		if (rect.area() < 30) continue;//小さいのを除外
		*/

		pupils.push_back(rect);
	}

	if (pupils.size() != 0)
	{
		std::sort(pupils.begin(), pupils.end(), [](Rect a, Rect b) -> bool { return a.area() < b.area(); });
		pupil = Point(pupils[0].x, pupils[0].y);

		return true;
	}

	return false;
}

void PupilFinder::emphasize(Mat img) {
	erode(img, img, Mat(erode_filter, erode_filter, CV_8UC1), Point(-1, -1), erode_itr); // 収縮(ノイズ除去)、対象ピクセルの近傍のうち最大
	dilate(img, img, Mat(dilate_filter, dilate_filter, CV_8UC1), Point(-1, -1), dilate_itr); // 膨張（強調）、対象ピクセルの近傍のうち最小
}

Rect PupilFinder::getEyeCrop(Point center, Point lidLeft, Point lidRight) {
	int base = abs((float)lidRight.x - lidLeft.x);
	Rect rectColor = Rect(center.x - base / 2 * 3 / 2, center.y - base / 2, base / 2 * 3, base);

	Mat eyeCoordColor = (Mat_<float>(3, 1) << center.x, center.y, 1);
	Mat eyeCoordIR = H * eyeCoordColor;

	Point eyePointIR = Point(eyeCoordIR.at<float>(0), eyeCoordIR.at<float>(1));
	Rect eyeRectIR = Rect(eyePointIR.x - base / 2 * 3 / 2, eyePointIR.y - base / 2, base / 2 * 3, base);
	
	return eyeRectIR;
}

bool PupilFinder::find(Mat ir, Mat color, map<FaceData::LandmarkType, Point> landmarks, Point &leftPupil, Point &rightPupil) {
	Mat ir1ch;
	cvtColor(ir, ir1ch, CV_BGR2GRAY);

	Mat irBinary;
	threshold(ir1ch, irBinary, thresh, 255, CV_THRESH_BINARY);

	Point leftEye = landmarks[LandmarkType::LANDMARK_EYE_LEFT_CENTER]; //左目の中心
	Point leftEyeLidL = landmarks[LandmarkType::LANDMARK_EYELID_LEFT_LEFT]; //左目瞼の左端
	Point leftEyeLidR = landmarks[LandmarkType::LANDMARK_EYELID_LEFT_RIGHT]; //左目瞼の右端

	Point rightEye = landmarks[LandmarkType::LANDMARK_EYE_RIGHT_CENTER]; //右目の中心
	Point rightEyeLidL = landmarks[LandmarkType::LANDMARK_EYELID_RIGHT_LEFT]; //右目瞼の左端
	Point rightEyeLidR = landmarks[LandmarkType::LANDMARK_EYELID_RIGHT_RIGHT]; //右目瞼の右端


	bool left = false;
	bool right = false;
	if (leftEye.x != -1) {
		Rect r = getEyeCrop(leftEye, leftEyeLidL, leftEyeLidR);
		if (r.area() > 0) {
			Mat leftEyeCrop = irBinary(r);
			emphasize(leftEyeCrop);

			left = findPupil(leftEyeCrop, leftPupil);
			if (DEBUG) {
				cv::rectangle(ir, r, cv::Scalar(255), 2);
			}

			leftPupil += Point(r.x, r.y);

			cv::imshow("leftEye", leftEyeCrop);
		}
	}

	if (rightEye.x != -1) {
		Rect r = getEyeCrop(rightEye, rightEyeLidL, rightEyeLidR);
		if (r.area() > 0) {
			Mat rightEyeCrop = irBinary(r);
			emphasize(rightEyeCrop);

			right = findPupil(rightEyeCrop, rightPupil);
			if (DEBUG) {
				cv::rectangle(ir, r, cv::Scalar(255), 2);
			}

			rightPupil += Point(r.x, r.y);

			cv::imshow("rightEye", rightEyeCrop);
		}
	}


	cv::imshow("binary", irBinary);
	if (DEBUG) {
		wirb << irBinary;
	}

	if (left || right) {
		return true;
	}

	return false;

}
