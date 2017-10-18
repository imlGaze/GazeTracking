#include <opencv2/opencv.hpp>
#include <RealSense/Face/FaceData.h>
#include "RealSenseAPI/RealSenseAPI.h"
#include "PreProcessor.h"
#include "MainProcessor.h"
#include "Stopwatch.h"
#include "const.h"
#include<map>

using std::vector;
using std::map;
using namespace std::chrono;
using namespace cv;
using Intel::RealSense::Face::FaceData;

double fps(long nano) {
	return 1000 / ((double)nano / 1000 / 1000);
}

map<LandmarkType, Point> landmarks;


int main() {
	RealSenseAPI realSense;
	if (!realSense.initialize(IMAGE_WIDTH, IMAGE_HEIGHT)) {
		std::cout << "ERROR: No device was found" << std::endl;
	}

	Mat colorColor(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3);
	Mat irGray(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3);
	Mat irBinary(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC1);

	Mat H = (Mat_<float>(3, 3) << 0.7351358246144999, -0.0319196931238943, 37.4838452084103,
		-0.008067602816914776, 0.7099678351704652, 74.76441033183021,
		-2.565364511844114e-05, -0.0001259361553461414, 1);

	FILE *feye = DEBUG ? fopen("eye.csv", "w") : nullptr;

	int fs = 3;
	int er = 2;
	int di = 3;

	Stopwatch fpsTimer;
	fpsTimer.start();
	map<LandmarkType, Point> landmarks;

	namedWindow("colorColor", CV_WINDOW_AUTOSIZE);
	namedWindow("irGray", CV_WINDOW_AUTOSIZE);
	
	VideoWriter wcol = DEBUG ? VideoWriter("color.avi", CV_FOURCC_DEFAULT, 30, cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), true) : VideoWriter();
	VideoWriter wir = DEBUG ? VideoWriter("ir.avi", CV_FOURCC_DEFAULT, 30, cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), true) : VideoWriter();

	while (1) {
		if (!realSense.queryNextFrame(irGray, colorColor, landmarks)) {
			// continue;
		}
		
		if (DEBUG) {
			wcol << colorColor;
			wir << irGray;
		}

		makeBinary(irGray, irBinary, 80);
		// emphasize(irBinary, irBinary);

		Point leftEye(-1, -1);
		Point leftEyeLidL(-1, -1);
		Point leftEyeLidR(-1, -1);

		Point rightEye(-1, -1);
		Point rightEyeLidL(-1, -1);
		Point rightEyeLidR(-1, -1);

		leftEye = landmarks[LandmarkType::LANDMARK_EYE_LEFT_CENTER]; //左目の中心
		leftEyeLidL = landmarks[LandmarkType::LANDMARK_EYELID_LEFT_LEFT]; //左目瞼の左端
		leftEyeLidR = landmarks[LandmarkType::LANDMARK_EYELID_LEFT_RIGHT]; //左目瞼の右端

		rightEye = landmarks[LandmarkType::LANDMARK_EYE_RIGHT_CENTER]; //右目の中心
		rightEyeLidL = landmarks[LandmarkType::LANDMARK_EYELID_RIGHT_LEFT]; //右目瞼の左端
		rightEyeLidR = landmarks[LandmarkType::LANDMARK_EYELID_RIGHT_RIGHT]; //右目瞼の右端

		RenderEye(colorColor, irGray, irBinary, H, leftEye, leftEyeLidL, leftEyeLidR);
		RenderEye(colorColor, irGray, irBinary, H, rightEye, rightEyeLidL, rightEyeLidR);

		if (DEBUG) {
			fprintf(feye, "%d,%d,%d,%d\n", leftEye.x, leftEye.y, rightEye.x, rightEye.y);
			fflush(feye);
		}

		if (leftEye.x != -1) {
			int base = abs((float)leftEyeLidR.x - leftEyeLidL.x);
			Rect rectColor = Rect(leftEye.x - base / 2 * 3 / 2, leftEye.y - base / 2, base / 2 * 3, base);
			rectangle(colorColor, rectColor, Scalar(255), 2);
			rectangle(colorColor, Rect(leftEye.x - 8, leftEye.y - 8, 16, 16), Scalar(0, 0, 255), 2);

			Mat leftEyeMatColor = (Mat_<float>(3, 1) << leftEye.x, leftEye.y, 1);
			Mat leftEyeMatIR = H * leftEyeMatColor;
			Point leftEyeIR = Point(leftEyeMatIR.at<float>(0), leftEyeMatIR.at<float>(1));

			Rect rectIR = Rect(leftEyeIR.x - base / 2 * 3 / 2, leftEyeIR.y - base / 2, base / 2 * 3, base);
			rectangle(irGray, rectIR, Scalar(255), 2);

			vector<Rect> pupils;
			if (findPupils(irBinary(rectIR), pupils)) {
				// std::sort(pupils.begin(), pupils.end(), [](Rect a, Rect b) { return a.area() < b.area(); });
				for (int i = 0, n = pupils.size(); i < n; i++) {
					rectangle(irGray, pupils[i] + Point(rectIR.x, rectIR.y), Scalar(0, 255), 2);
				}
			}


			Size eyeSize = rectIR.size();
			Mat eyeGray = Mat(128, 128, CV_8UC3);
			Mat eyeBinary = Mat(128, 128, CV_8UC1);
			irGray(rectIR).copyTo(eyeGray(Rect(0, 0, eyeSize.width, eyeSize.height)));
			irBinary(rectIR).copyTo(eyeBinary(Rect(0, 0, eyeSize.width, eyeSize.height)));
			imshow("eyeGray", eyeGray);
			imshow("eyeBinary", eyeBinary);
		}

		imshow("colorColor", colorColor);
		imshow("irGray", irGray);

		char key = waitKey(1);
		if (key == 'q') {
			break;
		}
	}

	if (DEBUG) {
		fclose(feye);
	}
	std::cout << "Total: " << fpsTimer.stop() << std::endl;

	return 0;

}

