#include <RealSense/Session.h>
#include <RealSense/SenseManager.h>
#include <RealSense/SampleReader.h>
#include <RealSense/Face/FaceModule.h>
#include <RealSense/Face/FaceData.h>
#include <RealSense/Face/FaceConfiguration.h>
#include<opencv2\opencv.hpp>

#include"RealSenseAPI.h"

using namespace cv;
using namespace Intel::RealSense;
using namespace Intel::RealSense::Face;

using std::vector;


void renderNoSignal(Mat &image) {
	image = Scalar(0); // �h��Ԃ�
	putText(image, "No signal", Point(30, 30), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(255), 1); // ������`��
}


bool RealSenseAPI::initialize(int imageWidth, int imageHeight, int fps)
{
	/* RealSense�̏����� */
	senseManager = SenseManager::CreateInstance();

	senseManager->EnableStream(Capture::STREAM_TYPE_IR, imageWidth, imageHeight, fps); // IR�J�����@�\��L����
	senseManager->EnableStream(Capture::STREAM_TYPE_COLOR, imageWidth, imageHeight, fps); // Color�J�����@�\��L����
	senseManager->EnableFace(); // FaceModule�@�\��L����

	senseManager->Init(); // �}�l�[�W���[�̏�����

	device = senseManager->QueryCaptureManager()->QueryDevice(); // RealSense�f�o�C�X�̎擾
	if (device != nullptr) // �f�o�C�X�����o�����ꍇ
	{
		device->ResetProperties(Capture::STREAM_TYPE_ANY);
		device->SetMirrorMode(Capture::Device::MirrorMode::MIRROR_MODE_DISABLED);
		device->SetIVCAMLaserPower(1); // IR���[�U�[�̏o�͂�ݒ�

		fmod = senseManager->QueryFace(); // FaceModule�̎擾
		fdata = fmod->CreateOutput(); // FaceModule�̏o�͐�̎擾

		fconfig = fmod->CreateActiveConfiguration(); // FaceModule�̐ݒ�
		fconfig->detection.isEnabled = true; // �猟�o��L����
		fconfig->detection.maxTrackedFaces = 3; // �ő�̊�ǐՐ� // TODO: 1?
		fconfig->landmarks.isEnabled = true; // ������_�̌��o��L����
		landmarkPoints = new FaceData::LandmarkPoint[fconfig->landmarks.numLandmarks]; // ������_������z���p��
		fconfig->ApplyChanges(); // �ݒ��K�p

		return true;
	}


	return false;
}


bool RealSenseAPI::queryNextFrame(Mat &irImage, Mat &colorImage, vector<FaceLandmark> &landmarks) {
	status = senseManager->AcquireFrame(true); // Frame���擾

	if (status < Status::STATUS_NO_ERROR) { // �G���[
		renderNoSignal(irImage);
		renderNoSignal(colorImage);
		landmarks.clear();
		senseManager->ReleaseFrame();
		return false;
	}

	bool successImage, successFace;
	if (!(successImage = queryImage(irImage, colorImage))) { // �J�����摜���擾
		renderNoSignal(irImage);
		renderNoSignal(colorImage);
	}

	landmarks.clear();
	successFace = queryFaceLandmarks(landmarks); // ������_���擾

	senseManager->ReleaseFrame();

	return successImage && successFace; // ���ׂĐ��������ꍇ�̂�true
}

void realSenseToCV(Image::ImageData src, Mat &dest) {
	memcpy(dest.data, src.planes[0], src.pitches[0] * dest.size().height); // RealSense�̉摜�f�[�^����OpenCV��Mat�Ƀf�[�^�𗬂�
}

bool RealSenseAPI::queryImage(Mat &irCV, Mat &colorCV)
{
	if (status < Status::STATUS_NO_ERROR) {
		return false;
	}

	const Capture::Sample *sample = senseManager->QuerySample();
	if (sample)
	{
		Image *colorRS = sample->color;
		Image *irRS = sample->ir;
		// Image *depthRS = projection->CreateDepthImageMappedToColor(sample->depth, sample->color);

		Image::ImageData colorData = {}; //={}�\���̂̏��������@
		Image::ImageData irData = {};

		Image::ImageInfo colorInfo = colorRS->QueryInfo();
		Image::ImageInfo irInfo = irRS->QueryInfo();

		// �摜�f�[�^�擾
		Status colorStatus = colorRS->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_RGB24, colorRS->QueryRotation(), Image::OPTION_ANY, &colorData);
		Status irStatus = irRS->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_Y16, irRS->QueryRotation(), Image::OPTION_ANY, &irData);
		if (colorStatus >= Status::STATUS_NO_ERROR && irStatus >= Status::STATUS_NO_ERROR) // �G���[�Ȃ�
		{
			realSenseToCV(colorData, colorCV);

			Mat irBuffer16UC1(irCV.size(), CV_16UC1);
			realSenseToCV(irData, irBuffer16UC1);
			Mat irBuffer8UC1(irCV.size(), CV_8UC1);
			irBuffer16UC1.convertTo(irBuffer8UC1, CV_8UC1);
			cvtColor(irBuffer8UC1, irCV, CV_GRAY2BGR); // 1ch(Y) -> 3ch(B, G, R)

													   // RealSense�̉摜�f�[�^�����
			colorRS->ReleaseAccess(&colorData);
			irRS->ReleaseAccess(&irData);

			// projection->Release();
			return true;
		}
		else
		{
			std::cout << "error" << std::endl;
			return false;
		}
	}
	else
	{
		std::cout << "sample is nullptr";
		return false;
	}
}

bool RealSenseAPI::queryFaceLandmarks(vector<FaceLandmark> &landmarks) {
	if (status < Status::STATUS_NO_ERROR) {
		return false;
	}

	fdata->Update(); // �猟�o�f�[�^���X�V
	
	for (int i = 0, n = fdata->QueryNumberOfDetectedFaces(); i < n; i++) { // ���o������̕��������[�v
		FaceData::Face *face = fdata->QueryFaceByIndex(i);

		if (face) { // �擾�ł��Ȃ������ꍇnull
			FaceData::LandmarksData *lmdata = face->QueryLandmarks(); // RealSense�̓����_�f�[�^���擾

			if (lmdata) { // �擾�ł��Ȃ������ꍇnull
				int numPoints = lmdata->QueryNumPoints(); // ���ۂɌ������������_�̐����擾
				if (numPoints == fconfig->landmarks.numLandmarks) { // �����_�̐����z�肵�Ă������ƈ�v
					lmdata->QueryPoints(landmarkPoints); // �����_�̏ڍׂȃf�[�^���擾

					if (landmarkPoints->confidenceImage) {
						for (int j = 0; j < numPoints; j++) {
							FaceData::LandmarkType alias = landmarkPoints[j].source.alias; // �����_�̃��x��
							int x = landmarkPoints[j].image.x; // �����_�̍��W
							int y = landmarkPoints[j].image.y;

							landmarks.push_back(FaceLandmark(alias, x, y));
						}

						return true;
					}
				}
			}
		}
	}

	return false;
}

