#include "MainProcessor.h"

#include <opencv2\opencv.hpp>

using namespace cv;
using std::vector;

bool findPupils(Mat binary, vector<Rect> &pupils) {
	vector<vector<Point>> contours;
	
	// �֊s(Contour)���o�ARETR_EXTERNAL�ōł��O���̂݁ACHAIN_APPROX_NONE�ł��ׂĂ̗֊s�_�i�֊s���\������_�j���i�[
	findContours(binary, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	for (int i = 0, n = contours.size(); i < n; i++)
	{
		Rect rect = boundingRect(contours[i]); // �_�̏W���ɊO�ڂ���X���Ă��Ȃ���`�����߂�
		pupils.push_back(rect);
	}

	return pupils.size() != 0;
}
