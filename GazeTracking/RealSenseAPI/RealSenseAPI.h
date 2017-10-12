#pragma once
#include <RealSense/Session.h>
#include <RealSense/SenseManager.h>
#include <RealSense/SampleReader.h>
#include <RealSense/Face/FaceModule.h>
#include <RealSense/Face/FaceData.h>
#include <RealSense/Face/FaceConfiguration.h>
#include<opencv2\opencv.hpp>

using namespace cv;
using namespace Intel::RealSense;
using namespace Intel::RealSense::Face;

using std::vector;

struct FaceLandmark {
	FaceData::LandmarkType type;
	int x, y;

	FaceLandmark(FaceData::LandmarkType type, int x, int y) {
		FaceLandmark::type = type;
		FaceLandmark::x = x;
		FaceLandmark::y = y;
	}
};

class RealSenseAPI
{
	bool RealSenseAPI::queryImage(Mat &irCV, Mat &colorCV);
	bool queryFaceLandmarks(vector<FaceLandmark> &landmarks);
public:
	bool initialize(int imageWidth, int imageHeight, int fps = 30);

	bool queryNextFrame(Mat &irImage, Mat &colorImage, vector<FaceLandmark> &landmarks);

	~RealSenseAPI() {
		senseManager->Release();
		delete landmarkPoints;
	};

private:
	SenseManager *senseManager;
	Capture::Device *device;
	Status status;

	FaceModule *fmod;
	FaceData *fdata;
	FaceConfiguration *fconfig;
	FaceData::LandmarkPoint *landmarkPoints;
};
