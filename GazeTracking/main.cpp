#include <opencv2/opencv.hpp>
#include "RealSenseAPI/RealSenseAPI.h"
#include <RealSense/Face/FaceData.h>
#include "const.h"

using std::vector;
using Intel::RealSense::Face::FaceData;

int main() {
	RealSenseAPI realSense;
	if (!realSense.initialize(IMAGE_WIDTH, IMAGE_HEIGHT)) {
		std::cout << "ERROR: No device was found" << std::endl;
	}

	Mat colorColor(480, 640, CV_8UC3);
	Mat irGray(480, 640, CV_8UC3);
	vector<FaceLandmark> landmarks;

	namedWindow("colorColor", CV_WINDOW_AUTOSIZE);
	namedWindow("irGray", CV_WINDOW_AUTOSIZE);

	while (1) {
		if (! realSense.queryNextFrame(irGray, colorColor, landmarks)) {
			// continue;
		}

		Point leftEye(-1, -1);
		for (int i = 0, n = landmarks.size(); i < n; i++) {
			FaceLandmark landmark = landmarks[i];

			if (landmark.type == LandmarkType::LANDMARK_EYE_LEFT_CENTER) {
				leftEye = Point(landmark.x, landmark.y);
			}
		}

		if (leftEye.x != -1) {
			// TODO: 
			rectangle(colorColor, Rect(leftEye.x - 8, leftEye.y - 8, 16, 16), Scalar(255), 2);
		}

		imshow("colorColor", colorColor);
		imshow("irGray", irGray);

		char key = waitKey(1);
		if (key == 'q') {
			break;
		}
	}
	
	return 0;
}
