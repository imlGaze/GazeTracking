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
using std::map;

class RealSenseAPI
{
	bool RealSenseAPI::queryImage(Mat &irCV, Mat &colorCV);
	bool queryFaceLandmarks(map<LandmarkType, Point> &landmarks);
public:
	bool initialize(int imageWidth, int imageHeight, int fps = 30);

	bool queryNextFrame(Mat &irImage, Mat &colorImage, map<LandmarkType, Point> &landmarks);

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
