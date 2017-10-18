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

void RenderEye(Mat color, Mat ir, Mat bin, Mat h, Point Eye, Point EyeLidL, Point EyeLidR) {

	if (Eye.x != -1) {
		int base = abs((float)EyeLidR.x - EyeLidL.x);
		Rect rectColor = Rect(Eye.x - base / 2 * 3 / 2, Eye.y - base / 2, base / 2 * 3, base);
		rectangle(color, rectColor, Scalar(255), 2);

		Mat leftEyeMatColor = (Mat_<float>(3, 1) << Eye.x, Eye.y, 1);
		Mat leftEyeMatIR = h * leftEyeMatColor;
		Point EyeIR = Point(leftEyeMatIR.at<float>(0), leftEyeMatIR.at<float>(1));

		Rect rectIR = Rect(EyeIR.x - base / 2 * 3 / 2, EyeIR.y - base / 2, base / 2 * 3, base);
		rectangle(ir, rectIR, Scalar(255), 2);

		//erode(irBinary(rectIR), irBinary(rectIR), element, Point(-1, -1), er); // 収縮(ノイズ除去)、対象ピクセルの近傍のうち最大
		//dilate(irBinary(rectIR), irBinary(rectIR), element, Point(-1, -1), di); // 膨張（強調）、対象ピクセルの近傍のうち最小

		vector<Rect> pupils;
		if (findPupils(bin(rectIR), pupils)) {
			// std::sort(pupils.begin(), pupils.end(), [](Rect a, Rect b) { return a.area() < b.area(); });
			for (int i = 0, n = pupils.size(); i < n; i++) {
				Rect pupil = pupils[i];
				// 周りのノイズを消す
				Point lt = Point(pupil.x, pupil.y);
				Point rt = Point(pupil.x + pupil.width, pupil.y);
				Point lb = Point(pupil.x, pupil.y + pupil.height);
				Point rb = Point(pupil.x + pupil.width, pupil.y + pupil.height);

				const int margin = 8; // 適当
				if (lt.x <= margin || lb.x <= margin)  continue;
				if (lt.y <= margin || rt.y <= margin)  continue;
				if (rectIR.width - margin <= rt.x || rectIR.width - margin <= rb.x)  continue;
				if (rectIR.height - margin <= lb.y || rectIR.height - margin <= rb.y) continue;
				//枠に被るやつを除外
				//if (pupil.width > pupil.height) continue; //横長を除外
				if (pupil.width * pupil.height < 30)continue;//小さいのを除外

				rectangle(ir, pupil + Point(rectIR.x, rectIR.y), Scalar(0, 255), 2);

				/*Size eyeSize = rectIR.size();
				Mat eyeGray = Mat(128, 128, CV_8UC3);
				Mat eyeBinary = Mat(128, 128, CV_8UC1);
				ir(rectIR).copyTo(eyeGray(Rect(0, 0, eyeSize.width, eyeSize.height)));
				bin(rectIR).copyTo(eyeBinary(Rect(0, 0, eyeSize.width, eyeSize.height)));
				
				imshow("eyeGray", eyeGray);
				imshow("eyeBinary", eyeBinary);
				*/
			}
		}
	}
}
