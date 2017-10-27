#include <opencv2/opencv.hpp>
#include <RealSense/Face/FaceData.h>
#include "RealSenseAPI/RealSenseAPI.h"
#include "Stopwatch.h"
#include "PupilFinder.h"
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

int main() {
	RealSenseAPI realSense;
	if (!realSense.initialize(IMAGE_WIDTH, IMAGE_HEIGHT)) {
		std::cout << "ERROR: No device was found" << std::endl;
	}

	Mat ir(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3);
	Mat color(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3);

	Mat H = (Mat_<float>(3, 3) << 0.7351358246144999, -0.0319196931238943, 37.4838452084103,
		-0.008067602816914776, 0.7099678351704652, 74.76441033183021,
		-2.565364511844114e-05, -0.0001259361553461414, 1);

	FILE *feye = DEBUG ? fopen("eye.csv", "w") : nullptr;

	Stopwatch fpsTimer;
	fpsTimer.start();
	map<LandmarkType, Point> landmarks;

	namedWindow("ir", CV_WINDOW_AUTOSIZE);
	namedWindow("color", CV_WINDOW_AUTOSIZE);
	
	VideoWriter wir = DEBUG ? VideoWriter("ir.avi", CV_FOURCC_DEFAULT, 30, cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), true) : VideoWriter();
	VideoWriter wcol = DEBUG ? VideoWriter("color.avi", CV_FOURCC_DEFAULT, 30, cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), true) : VideoWriter();

	PupilFinder finder;

	while (1) {
		if (!realSense.queryNextFrame(ir, color, landmarks)) {
			// continue;
		}
		
		if (DEBUG) {
			wcol << color;
			wir << ir;
		}

		imshow("color", color);
		imshow("ir", ir);

		char key2 = waitKey(1);
		if (key2 == 'q') {
			break;
		}

		continue;

		Point leftPupil, rightPupil;
		if (finder.find(ir, color, landmarks, leftPupil, rightPupil)) {
			unsigned char data[] = {
				(leftPupil.x >> 24) & 0xFF, (leftPupil.x >> 16) & 0xFF, (leftPupil.x >> 8) & 0xFF, leftPupil.x & 0xFF,
				(leftPupil.y >> 24) & 0xFF, (leftPupil.y >> 16) & 0xFF, (leftPupil.y >> 8) & 0xFF, leftPupil.y & 0xFF,
				(rightPupil.x >> 24) & 0xFF, (rightPupil.x >> 16) & 0xFF, (rightPupil.x >> 8) & 0xFF, rightPupil.x & 0xFF,
				(rightPupil.y >> 24) & 0xFF, (rightPupil.y >> 16) & 0xFF, (rightPupil.y >> 8) & 0xFF, rightPupil.y & 0xFF,
			};

			if (DEBUG) {
				rectangle(color, Rect(leftPupil.x - 8, leftPupil.y - 8, 16, 16), Scalar(255), 2);
				rectangle(color, Rect(rightPupil.x - 8, rightPupil.y - 8, 16, 16), Scalar(0, 255), 2);
			}
		}

		if (DEBUG) {
			Point leftEye = landmarks[LandmarkType::LANDMARK_EYE_LEFT_CENTER];
			Point rightEye = landmarks[LandmarkType::LANDMARK_EYE_RIGHT_CENTER];

			fprintf(feye, "%d,%d,%d,%d\n", leftEye.x, leftEye.y, rightEye.x, rightEye.y);
			fflush(feye);
		}

		imshow("color", color);
		imshow("ir", ir);

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

