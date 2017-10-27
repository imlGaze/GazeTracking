#include <opencv2/opencv.hpp>
#include <RealSense/Face/FaceData.h>
#include "RealSenseAPI/RealSenseAPI.h"
#include "Stopwatch.h"
#include "PupilFinder.h"
#include "UDP/UDP.h"
#include "const.h"
#include <map>

using std::vector;
using std::map;
using namespace std::chrono;
using Intel::RealSense::Face::FaceData;
using cv::Mat;
using cv::Scalar;
using cv::Point;
using cv::Size;
using cv::VideoWriter;
using cv::namedWindow;
using cv::waitKey;

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

	FILE *feye = DEBUG ? fopen("eye.csv", "w") : nullptr;

	Stopwatch fpsTimer;
	fpsTimer.start();
	map<LandmarkType, Point> landmarks;

	namedWindow("ir", CV_WINDOW_AUTOSIZE);
	namedWindow("color", CV_WINDOW_AUTOSIZE);
	
	VideoWriter wir = DEBUG ? VideoWriter("ir.avi", CV_FOURCC_DEFAULT, 30, Size(IMAGE_WIDTH, IMAGE_HEIGHT), true) : VideoWriter();
	VideoWriter wcol = DEBUG ? VideoWriter("color.avi", CV_FOURCC_DEFAULT, 30, Size(IMAGE_WIDTH, IMAGE_HEIGHT), true) : VideoWriter();

	UDP udp(31416, "127.0.0.1");
	PupilFinder finder;

	while (1) {
		if (!realSense.queryNextFrame(ir, color, landmarks)) {
			// continue;
		}
		
		Point leftPupil, rightPupil;
		finder.find(ir, color, landmarks, leftPupil, rightPupil);
		unsigned char data[] = {
			(leftPupil.x >> 24) & 0xFF, (leftPupil.x >> 16) & 0xFF, (leftPupil.x >> 8) & 0xFF, leftPupil.x & 0xFF,
			(leftPupil.y >> 24) & 0xFF, (leftPupil.y >> 16) & 0xFF, (leftPupil.y >> 8) & 0xFF, leftPupil.y & 0xFF,
			(rightPupil.x >> 24) & 0xFF, (rightPupil.x >> 16) & 0xFF, (rightPupil.x >> 8) & 0xFF, rightPupil.x & 0xFF,
			(rightPupil.y >> 24) & 0xFF, (rightPupil.y >> 16) & 0xFF, (rightPupil.y >> 8) & 0xFF, rightPupil.y & 0xFF,
		};

		udp.send(data, sizeof data); // TODO: needs timeout? sometimes freeze!


		if (DEBUG) {
			if (leftPupil.x > 8 && leftPupil.y > 8) {
				rectangle(ir, Rect(leftPupil.x - 8, leftPupil.y - 8, 16, 16), Scalar(0, 0, 255), 2);
			}
			if (rightPupil.x > 8 && rightPupil.y > 8) {
				rectangle(ir, Rect(rightPupil.x - 8, rightPupil.y - 8, 16, 16), Scalar(0, 0, 255), 2);
			}
		}

		if (DEBUG) {
			Point leftEye = landmarks[LandmarkType::LANDMARK_EYE_LEFT_CENTER];
			Point rightEye = landmarks[LandmarkType::LANDMARK_EYE_RIGHT_CENTER];

			fprintf(feye, "%d,%d,%d,%d\n", leftEye.x, leftEye.y, rightEye.x, rightEye.y);
			fflush(feye);
		}

		if (DEBUG) {
			wcol << color;
			wir << ir;
		}

		imshow("color", color);
		imshow("ir", ir);

		long lap = fpsTimer.lap();

		char key = waitKey(1);
		if (key == 'q') {
			break;
		}
		else if (key == 't') {
			finder.thresh++;
			std::cout << "Thresh: " << finder.thresh << std::endl;
		}
		else if (key == 'T') {
			finder.thresh--;
			std::cout << "Thresh: " << finder.thresh << std::endl;
		}
		else if (key == 'e') {
			finder.erode_itr++;
			std::cout << "Erode: " << finder.erode_itr << std::endl;
		}
		else if (key == 'E') {
			finder.erode_itr--;
			std::cout << "Erode: " << finder.erode_itr << std::endl;
		}
		else if (key == 'd') {
			finder.dilate_itr++;
			std::cout << "Dilate: " << finder.dilate_itr << std::endl;
		}
		else if (key == 'D') {
			finder.dilate_itr--;
			std::cout << "Dilate: " << finder.dilate_itr << std::endl;
		}
		else if (key == 'f') {
			std::cout << "FPS: " << fps(lap) << std::endl;
		}

	}

	if (DEBUG) {
		fclose(feye);
	}
	std::cout << "Total: " << fpsTimer.stop() << std::endl;

	return 0;

}

