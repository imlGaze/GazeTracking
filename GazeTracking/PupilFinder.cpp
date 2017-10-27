#include <opencv2/opencv.hpp>
#include <RealSense/Face/FaceData.h>

#include "PupilFinder.h"
#include "const.h"

using namespace cv;
using namespace Intel::RealSense;
using namespace Intel::RealSense::Face;

using std::vector;

bool PupilFinder::findPupil(Mat crop, Point &pupil) {
	vector<vector<Point>> contours;
	// �֊s(Contour)���o�ARETR_EXTERNAL�ōł��O���̂݁ACHAIN_APPROX_NONE�ł��ׂĂ̗֊s�_�i�֊s���\������_�j���i�[
	findContours(crop, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	Size size = crop.size();
	vector<Rect> pupils;

	for (int i = 0, n = contours.size(); i < n; i++)
	{
		Rect rect = boundingRect(contours[i]); // �_�̏W���ɊO�ڂ���X���Ă��Ȃ���`�����߂�

		Point lt = Point(rect.x, rect.y);
		Point rt = Point(rect.x + rect.width, rect.y);
		Point lb = Point(rect.x, rect.y + rect.height);
		Point rb = Point(rect.x + rect.width, rect.y + rect.height);

		const int margin = 8; // �K��
		if (lt.x <= margin || lb.x <= margin)  continue;
		if (lt.y <= margin || rt.y <= margin)  continue;
		if (size.width - margin <= rt.x || size.width - margin <= rb.x)  continue;
		if (size.height - margin <= lb.y || size.height - margin <= rb.y) continue;
		//�g�ɔ�������O
		//if (rect.width > rect.height) continue; //���������O
		if (rect.area() < 30)continue;//�������̂����O

		pupils.push_back(rect);
	}

	return pupils.size() != 0;
}

void PupilFinder::emphasize(Mat img) {
	erode(img, img, Mat(erode_filter, erode_filter, CV_8UC1), Point(-1, -1), erode_itr); // ���k(�m�C�Y����)�A�Ώۃs�N�Z���̋ߖT�̂����ő�
	dilate(img, img, Mat(dilate_filter, dilate_filter, CV_8UC1), Point(-1, -1), dilate_itr); // �c���i�����j�A�Ώۃs�N�Z���̋ߖT�̂����ŏ�
}

Rect PupilFinder::getEyeCrop(Point center, Point lidLeft, Point lidRight) {
	int base = abs((float)lidRight.x - lidLeft.x);
	Rect rectColor = Rect(center.x - base / 2 * 3 / 2, center.y - base / 2, base / 2 * 3, base);

	Mat eyeCoordColor = (Mat_<float>(3, 1) << center.x, center.y, 1);
	Mat eyeCoordIR = H * eyeCoordColor;

	Point eyePointIR = Point(eyeCoordIR.at<float>(0), eyeCoordIR.at<float>(1));
	Rect eyeRectIR = Rect(eyePointIR.x - base / 2 * 3 / 2, eyePointIR.y - base / 2, base / 2 * 3, base);
	
	return eyeRectIR;
}

bool PupilFinder::find(Mat ir, Mat color, map<FaceData::LandmarkType, Point> landmarks, Point &leftPupil, Point &rightPupil) {
	Mat ir1ch;
	cvtColor(ir, ir1ch, CV_BGR2GRAY);

	Mat irBinary;
	threshold(ir1ch, irBinary, thresh, 255, CV_THRESH_BINARY);

	Point leftEye = landmarks[LandmarkType::LANDMARK_EYE_LEFT_CENTER]; //���ڂ̒��S
	Point leftEyeLidL = landmarks[LandmarkType::LANDMARK_EYELID_LEFT_LEFT]; //�����ق̍��[
	Point leftEyeLidR = landmarks[LandmarkType::LANDMARK_EYELID_LEFT_RIGHT]; //�����ق̉E�[

	Point rightEye = landmarks[LandmarkType::LANDMARK_EYE_RIGHT_CENTER]; //�E�ڂ̒��S
	Point rightEyeLidL = landmarks[LandmarkType::LANDMARK_EYELID_RIGHT_LEFT]; //�E���ق̍��[
	Point rightEyeLidR = landmarks[LandmarkType::LANDMARK_EYELID_RIGHT_RIGHT]; //�E���ق̉E�[


	bool left = false;
	bool right = false;
	if (leftEye.x != -1) {
		Mat leftEyeCrop = irBinary(getEyeCrop(leftEye, leftEyeLidL, leftEyeLidR));
		emphasize(leftEyeCrop);

		left = findPupil(leftEyeCrop, leftPupil);
	}

	if (rightEye.x != -1) {
		Mat rightEyeCrop = irBinary(getEyeCrop(rightEye, rightEyeLidL, rightEyeLidR));
		emphasize(rightEyeCrop);

		right = findPupil(rightEyeCrop, rightPupil);
	}

	if (left || right) {
		return true;
	}

	return false;

}
