#include <opencv2/opencv.hpp>
#include <RealSense/Face/FaceData.h>

#include "PupilFinder.h"
#include "const.h"

using namespace cv;
using namespace Intel::RealSense;
using namespace Intel::RealSense::Face;

using std::vector;

bool PupilFinder::find(Mat ir, Mat color, map<FaceData::LandmarkType, Point> landmarks, vector<Point> &pupils) {
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


	if (leftEye.x != -1) {
		int base = abs((float)leftEyeLidR.x - leftEyeLidL.x);
		Rect rectColor = Rect(leftEye.x - base / 2 * 3 / 2, leftEye.y - base / 2, base / 2 * 3, base);
		Mat leftEyeMatColor = (Mat_<float>(3, 1) << leftEye.x, leftEye.y, 1);
		Mat leftEyeMatIR = H * leftEyeMatColor;
		Point leftEyeIR = Point(leftEyeMatIR.at<float>(0), leftEyeMatIR.at<float>(1));
		Rect rectIR = Rect(leftEyeIR.x - base / 2 * 3 / 2, leftEyeIR.y - base / 2, base / 2 * 3, base);

		if (DEBUG) {
			rectangle(color, rectColor, Scalar(255), 2);
			rectangle(color, Rect(leftEye.x - 8, leftEye.y - 8, 16, 16), Scalar(0, 0, 255), 2);
			rectangle(ir, rectIR, Scalar(255), 2);
		}


		Mat irBinaryCrop = irBinary(rectIR);

		erode(irBinaryCrop, irBinaryCrop, Mat(erode_filter, erode_filter, CV_8UC1), Point(-1, -1), erode_itr); // 収縮(ノイズ除去)、対象ピクセルの近傍のうち最大
		dilate(irBinaryCrop, irBinaryCrop, Mat(dilate_filter, dilate_filter, CV_8UC1), Point(-1, -1), dilate_itr); // 膨張（強調）、対象ピクセルの近傍のうち最小

		vector<vector<Point>> contours;
		// 輪郭(Contour)抽出、RETR_EXTERNALで最も外側のみ、CHAIN_APPROX_NONEですべての輪郭点（輪郭を構成する点）を格納
		findContours(irBinaryCrop, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);


		for (int i = 0, n = contours.size(); i < n; i++)
		{
			Rect rect = boundingRect(contours[i]); // 点の集合に外接する傾いていない矩形を求める

			Point lt = Point(rect.x, rect.y);
			Point rt = Point(rect.x + rect.width, rect.y);
			Point lb = Point(rect.x, rect.y + rect.height);
			Point rb = Point(rect.x + rect.width, rect.y + rect.height);

			const int margin = 8; // 適当
			if (lt.x <= margin || lb.x <= margin)  continue;
			if (lt.y <= margin || rt.y <= margin)  continue;
			if (rectIR.width - margin <= rt.x || rectIR.width - margin <= rb.x)  continue;
			if (rectIR.height - margin <= lb.y || rectIR.height - margin <= rb.y) continue;
			//枠に被るやつを除外
			//if (rect.width > rect.height) continue; //横長を除外
			if (rect.area() < 30)continue;//小さいのを除外

			pupils.push_back(Point(rect.x, rect.y));
		}

		// std::sort(pupils.begin(), pupils.end(), [](Rect a, Rect b) { return a.area() < b.area(); });
		if (DEBUG) {
			for (int i = 0, n = pupils.size(); i < n; i++) {
				rectangle(ir, Rect(pupils[i].x - 8, pupils[i].y - 8, 16, 16) + Point(rectIR.x, rectIR.y), Scalar(0, 255), 2);
			}
		}

		/*
		if (DEBUG) {
			RenderEye(color, ir, irBinary, H, leftEye, leftEyeLidL, leftEyeLidR);
			RenderEye(color, ir, irBinary, H, rightEye, rightEyeLidL, rightEyeLidR);
		}
		*/

		if (DEBUG) {
			Size eyeSize = rectIR.size();
			Mat eyeGray = Mat(128, 128, CV_8UC3);
			Mat eyeBinary = Mat(128, 128, CV_8UC1);
			ir(rectIR).copyTo(eyeGray(Rect(0, 0, eyeSize.width, eyeSize.height)));
			irBinary(rectIR).copyTo(eyeBinary(Rect(0, 0, eyeSize.width, eyeSize.height)));
			imshow("eyeGray", eyeGray);
			imshow("eyeBinary", eyeBinary);
		}
	}

	return pupils.size() != 0;

}
