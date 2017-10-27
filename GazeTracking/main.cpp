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

	Mat color(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3);
	Mat ir(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3);

	Mat H = (Mat_<float>(3, 3) << 0.7351358246144999, -0.0319196931238943, 37.4838452084103,
		-0.008067602816914776, 0.7099678351704652, 74.76441033183021,
		-2.565364511844114e-05, -0.0001259361553461414, 1);

	FILE *feye = DEBUG ? fopen("eye.csv", "w") : nullptr;

	Stopwatch fpsTimer;
	fpsTimer.start();
	map<LandmarkType, Point> landmarks;

	namedWindow("color", CV_WINDOW_AUTOSIZE);
	namedWindow("ir", CV_WINDOW_AUTOSIZE);
	
	VideoWriter wcol = DEBUG ? VideoWriter("color.avi", CV_FOURCC_DEFAULT, 30, cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), true) : VideoWriter();
	VideoWriter wir = DEBUG ? VideoWriter("ir.avi", CV_FOURCC_DEFAULT, 30, cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), true) : VideoWriter();

	PupilFinder finder;

	while (1) {
		if (!realSense.queryNextFrame(ir, color, landmarks)) {
			// continue;
		}
		
		if (DEBUG) {
			wcol << color;
			wir << ir;
		}

		vector<Point> pupils;
		if (finder.find(ir, color, landmarks, pupils)) {

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

