#include <opencv2/opencv.hpp>
#include <RealSense/Face/FaceData.h>
#include "RealSenseAPI/RealSenseAPI.h"
#include "PreProcessor.h"
#include "MainProcessor.h"
#include "Stopwatch.h"
#include "const.h"

using std::vector;
using namespace std::chrono;
using Intel::RealSense::Face::FaceData;

double fps(long nano) {
	return 1000 / ((double) nano / 1000 / 1000);
}

int main() {
	RealSenseAPI realSense;
	if (!realSense.initialize(IMAGE_WIDTH, IMAGE_HEIGHT)) {
		std::cout << "ERROR: No device was found" << std::endl;
	}

	Mat colorColor(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3);
	Mat irGray(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3);
	Mat irBinary(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC1);
	Mat H = (Mat_<float>(3, 3) <<	0.7351358246144999, -0.0319196931238943, 37.4838452084103,
									-0.008067602816914776, 0.7099678351704652, 74.76441033183021,
									-2.565364511844114e-05, -0.0001259361553461414, 1);

	Stopwatch fpsTimer;
	fpsTimer.start();
	vector<FaceLandmark> landmarks;

	namedWindow("colorColor", CV_WINDOW_AUTOSIZE);
	namedWindow("irGray", CV_WINDOW_AUTOSIZE);

	while (1) {
		if (! realSense.queryNextFrame(irGray, colorColor, landmarks)) {
			// continue;
		}

		makeBinary(irGray, irBinary, 100);
		emphasize(irBinary, irBinary);
		
		Point leftEye(-1, -1); 
		Point leftEyeLidL(-1, -1);
		Point leftEyeLidR(-1, -1);

		
		for (int i = 0, n = landmarks.size(); i < n; i++) {
			FaceLandmark landmark = landmarks[i];

			if (landmark.type == LandmarkType::LANDMARK_EYE_LEFT_CENTER) {//左目の中心
				leftEye = Point(landmark.x, landmark.y);
			}

			if (landmark.type == LandmarkType::LANDMARK_EYELID_LEFT_LEFT) {//左目の左端
				leftEyeLidL = Point(landmark.x, landmark.y);
			}

			if (landmark.type == LandmarkType::LANDMARK_EYELID_LEFT_RIGHT) {//左目の右端
				leftEyeLidR = Point(landmark.x, landmark.y);
			}
		}

		if (leftEye.x != -1) {
			// TODO: 
			int base = abs((float) leftEyeLidR.x - leftEyeLidL.x);
			Rect rectColor = Rect(leftEye.x - base / 2 * 3 / 2, leftEye.y - base / 2, base / 2 * 3, base);
			rectangle(colorColor, rectColor, Scalar(255), 2);

			Mat leftEyeMatColor = (Mat_<float>(3, 1) << leftEye.x, leftEye.y, 1);
			Mat leftEyeMatIR = H * leftEyeMatColor;
			Point leftEyeIR = Point(leftEyeMatIR.at<float>(0), leftEyeMatIR.at<float>(1));

			Rect rectIR = Rect(leftEyeIR.x - base / 2 * 3 / 2, leftEyeIR.y - base / 2, base / 2 * 3, base);
			rectangle(irGray, rectIR, Scalar(255), 2);

			vector<Rect> pupils;
			if (findPupils(irBinary(rectIR), pupils)) {
				for (int i = 0, n = pupils.size(); i < n; i++) {
					rectangle(irGray, pupils[i] + Point(rectIR.x, rectIR.y), Scalar(0, 255), 2);
				}
			}
		}

		imshow("colorColor", colorColor);
		imshow("irGray", irGray);


		std::cout << "FPS: " << fps(fpsTimer.lap()) << std::endl;

		char key = waitKey(1);
		if (key == 'q') {
			break;
		}
	}

	std::cout << "Total: " << fpsTimer.stop() << std::endl;
	
	return 0;
}
