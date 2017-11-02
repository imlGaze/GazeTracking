#include <opencv2/opencv.hpp>
#include <RealSense/Face/FaceData.h>
#include "RealSenseAPI/RealSenseAPI.h"
#include "Stopwatch.h"
#include "PupilFinder.h"
#include "UDP/UDP.h"
#include "const.h"
#include <map>
#include <thread>
#include <atomic>
#include <random>

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
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		std::cout << "WSAStartup Error" << std::endl;
	}


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

	PupilFinder finder;

	Rect leftEyeRect, rightEyeRect;
	Point leftPupil, rightPupil;
	UDP udp(31416, "127.0.0.1");

	bool running = true;
	std::thread udpThread([&running, &leftPupil, &rightPupil]() -> void {
		UDP udp(31416, "127.0.0.1");

		while (running) {
			int lx = leftPupil.x;
			int ly = leftPupil.y;
			int rx = rightPupil.x;
			int ry = rightPupil.y;

			unsigned char data[] = {
				(lx >> 24) & 0xFF, (lx >> 16) & 0xFF, (lx >> 8) & 0xFF, lx & 0xFF,
				(ly >> 24) & 0xFF, (ly >> 16) & 0xFF, (ly >> 8) & 0xFF, ly & 0xFF,
				(rx >> 24) & 0xFF, (rx >> 16) & 0xFF, (rx >> 8) & 0xFF, rx & 0xFF,
				(ry >> 24) & 0xFF, (ry >> 16) & 0xFF, (ry >> 8) & 0xFF, ry & 0xFF,
			};

			bool rs = udp.send(data, sizeof data);
			
			if (DEBUG) {
				if (rs) {
					// std::cout << "Pupil: " << leftPupil << ", " << rightPupil << std::endl;
				}
				else {
					// std::cout << "Error" << std::endl;
				}
			}
			
			// Sleep(32);
		}
	});

	std::random_device rnd;
	int i = 0;
	while (1) {
		if (!realSense.queryNextFrame(ir, color, landmarks)) {
			// continue;
		}
		
		finder.find(ir, color, landmarks, leftEyeRect, rightEyeRect, leftPupil, rightPupil);

		if (DEBUG) {
			if (leftPupil.x > 8 && leftPupil.y > 8) {
				rectangle(ir, Rect(leftPupil.x - 8, leftPupil.y - 8, 16, 16) + Point(leftEyeRect.x, leftEyeRect.y), Scalar(0, 0, 255), 2);
			}
			if (rightPupil.x > 8 && rightPupil.y > 8) {
				rectangle(ir, Rect(rightPupil.x - 8, rightPupil.y - 8, 16, 16) + Point(rightEyeRect.x, rightEyeRect.y), Scalar(0, 0, 255), 2);
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
		else if (key == 'r') {
			if (!realSense.initialize(IMAGE_WIDTH, IMAGE_HEIGHT)) {
				std::cout << "Failed to re-init." << (i++ > 3 ? " R.I.P." : "") << std::endl;
			}
			else {
				std::cout << "Re-inited" << std::endl;
			}
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
		else if (key == 'r') {
			int r1 = rnd();
			int r2 = rnd();
			int r3 = rnd();
			int r4 = rnd();

			unsigned char data[] = {
				(r1 >> 24) & 0xFF, (r1 >> 16) & 0xFF, (r1 >> 8) & 0xFF, r1 & 0xFF,
				(r2 >> 24) & 0xFF, (r2 >> 16) & 0xFF, (r2 >> 8) & 0xFF, r2 & 0xFF,
				(r3 >> 24) & 0xFF, (r3 >> 16) & 0xFF, (r3 >> 8) & 0xFF, r3 & 0xFF,
				(r4 >> 24) & 0xFF, (r4 >> 16) & 0xFF, (r4 >> 8) & 0xFF, r4 & 0xFF,
			};
			udp.send(data, sizeof data);
			std::cout << std::hex << r1 << r2 << r3 << r4 << std::dec << std::endl;
		}
	}

	running = false;
	udpThread.detach(); // TODO: ?
	if (DEBUG) {
		fclose(feye);
	}
	std::cout << "Total: " << fpsTimer.stop() << std::endl;
	cv::destroyAllWindows();

	WSACleanup();
	return 0;

}

