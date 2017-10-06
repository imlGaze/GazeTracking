#include <opencv2/opencv.hpp>
#include <RealSense/Face/FaceData.h>
#include "RealSenseAPI/RealSenseAPI.h"
#include "PreProcessor.h"
#include "MainProcessor.h"
#include "Stopwatch.h"
#include "const.h"

using std::vector;
using namespace std::chrono;
using namespace cv;
using Intel::RealSense::Face::FaceData;

double fps(long nano) {
	return 1000 / ((double)nano / 1000 / 1000);
}

int main() {
	RealSenseAPI realSense;
	if (!realSense.initialize(IMAGE_WIDTH, IMAGE_HEIGHT)) {
		std::cout << "ERROR: No device was found" << std::endl;
	}

	Mat colorColor(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3);
	Mat irGray(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3);
	Mat irBinary(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC1);

	FILE *feye = DEBUG ? fopen("eye.csv", "w") : nullptr;

	int fs = 3;
	int er = 2;
	int di = 3;

	Stopwatch fpsTimer;
	fpsTimer.start();
	vector<FaceLandmark> landmarks;

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

		makeBinary(irGray, irBinary, 100);
		// emphasize(irBinary, irBinary);
		
		Mat element(fs, fs, CV_8UC1); // フィルタサイズ
		erode(irBinary, irBinary, element, Point(-1, -1), er); // 収縮(ノイズ除去)、対象ピクセルの近傍のうち最大
		dilate(irBinary, irBinary, element, Point(-1, -1), di); // 膨張（強調）、対象ピクセルの近傍のうち最小

		Point leftEye(-1, -1);
		Point rightEye(-1, -1);
		Point leftEyeLidL(-1, -1);
		Point leftEyeLidR(-1, -1);

		for (int i = 0, n = landmarks.size(); i < n; i++) {
			FaceLandmark landmark = landmarks[i];

			if (landmark.type == LandmarkType::LANDMARK_EYE_LEFT_CENTER) {//左目の中心
				leftEye = Point(landmark.x, landmark.y);
			}

			if (landmark.type == LandmarkType::LANDMARK_EYE_RIGHT_CENTER) {//左目の中心
				rightEye = Point(landmark.x, landmark.y);
			}

			if (landmark.type == LandmarkType::LANDMARK_EYELID_LEFT_LEFT) {//左目まぶたの左端
				leftEyeLidL = Point(landmark.x, landmark.y);
			}

			if (landmark.type == LandmarkType::LANDMARK_EYELID_LEFT_RIGHT) {//左目まぶたの右端
				leftEyeLidR = Point(landmark.x, landmark.y);
			}
		}

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

		cv::imshow("colorColor", colorColor);
		cv::imshow("irGray", irGray);

		// std::cout << "FPS: " << fps(fpsTimer.lap()) << std::endl;

		char key = waitKey(1);
		if (key == 'q') {
			break;
		}
		else if (key == 'f') {
			fs++;
			std::cout << "Filter: " << fs << std::endl;
		}
		else if (key == 'F') {
			if (fs > 0) {
				fs--;
				std::cout << "Filter: " << fs << std::endl;
			}
		}
		else if (key == 'e') {
			er++;
			std::cout << "Erode: " << er << std::endl;
		}
		else if (key == 'E') {
			if (er > 0) {
				er--;
				std::cout << "Erode: " << er << std::endl;
			}
		}
		else if (key == 'd') {
			di++;
			std::cout << "Dilate: " << di << std::endl;
		}
		else if (key == 'D') {
			if (di > 0) {
				di--;
				std::cout << "Dilate: " << di << std::endl;
			}
		}
	}

	if (DEBUG) {
		fclose(feye);
	}
	std::cout << "Total: " << fpsTimer.stop() << std::endl;
	
	return 0;
}
