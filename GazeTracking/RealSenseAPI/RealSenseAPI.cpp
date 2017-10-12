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
	image = Scalar(0); // 塗りつぶし
	putText(image, "No signal", Point(30, 30), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(255), 1); // 文字列描画
}


bool RealSenseAPI::initialize(int imageWidth, int imageHeight, int fps)
{
	/* RealSenseの初期化 */
	senseManager = SenseManager::CreateInstance();

	senseManager->EnableStream(Capture::STREAM_TYPE_IR, imageWidth, imageHeight, fps); // IRカメラ機能を有効化
	senseManager->EnableStream(Capture::STREAM_TYPE_COLOR, imageWidth, imageHeight, fps); // Colorカメラ機能を有効化
	senseManager->EnableFace(); // FaceModule機能を有効化

	senseManager->Init(); // マネージャーの初期化

	device = senseManager->QueryCaptureManager()->QueryDevice(); // RealSenseデバイスの取得
	if (device != nullptr) // デバイスを検出した場合
	{
		device->ResetProperties(Capture::STREAM_TYPE_ANY);
		device->SetMirrorMode(Capture::Device::MirrorMode::MIRROR_MODE_DISABLED);
		device->SetIVCAMLaserPower(1); // IRレーザーの出力を設定

		fmod = senseManager->QueryFace(); // FaceModuleの取得
		fdata = fmod->CreateOutput(); // FaceModuleの出力先の取得

		fconfig = fmod->CreateActiveConfiguration(); // FaceModuleの設定
		fconfig->detection.isEnabled = true; // 顔検出を有効化
		fconfig->detection.maxTrackedFaces = 3; // 最大の顔追跡数 // TODO: 1?
		fconfig->landmarks.isEnabled = true; // 顔特徴点の検出を有効化
		landmarkPoints = new FaceData::LandmarkPoint[fconfig->landmarks.numLandmarks]; // 顔特徴点を入れる配列を用意
		fconfig->ApplyChanges(); // 設定を適用

		return true;
	}


	return false;
}


bool RealSenseAPI::queryNextFrame(Mat &irImage, Mat &colorImage, vector<FaceLandmark> &landmarks) {
	status = senseManager->AcquireFrame(true); // Frameを取得

	if (status < Status::STATUS_NO_ERROR) { // エラー
		renderNoSignal(irImage);
		renderNoSignal(colorImage);
		landmarks.clear();
		senseManager->ReleaseFrame();
		return false;
	}

	bool successImage, successFace;
	if (!(successImage = queryImage(irImage, colorImage))) { // カメラ画像を取得
		renderNoSignal(irImage);
		renderNoSignal(colorImage);
	}

	landmarks.clear();
	successFace = queryFaceLandmarks(landmarks); // 顔特徴点を取得

	senseManager->ReleaseFrame();

	return successImage && successFace; // すべて成功した場合のみtrue
}

void realSenseToCV(Image::ImageData src, Mat &dest) {
	memcpy(dest.data, src.planes[0], src.pitches[0] * dest.size().height); // RealSenseの画像データからOpenCVのMatにデータを流す
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

		Image::ImageData colorData = {}; //={}構造体の初期化方法
		Image::ImageData irData = {};

		Image::ImageInfo colorInfo = colorRS->QueryInfo();
		Image::ImageInfo irInfo = irRS->QueryInfo();

		// 画像データ取得
		Status colorStatus = colorRS->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_RGB24, colorRS->QueryRotation(), Image::OPTION_ANY, &colorData);
		Status irStatus = irRS->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_Y16, irRS->QueryRotation(), Image::OPTION_ANY, &irData);
		if (colorStatus >= Status::STATUS_NO_ERROR && irStatus >= Status::STATUS_NO_ERROR) // エラーなし
		{
			realSenseToCV(colorData, colorCV);

			Mat irBuffer16UC1(irCV.size(), CV_16UC1);
			realSenseToCV(irData, irBuffer16UC1);
			Mat irBuffer8UC1(irCV.size(), CV_8UC1);
			irBuffer16UC1.convertTo(irBuffer8UC1, CV_8UC1);
			cvtColor(irBuffer8UC1, irCV, CV_GRAY2BGR); // 1ch(Y) -> 3ch(B, G, R)

													   // RealSenseの画像データを解放
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

	fdata->Update(); // 顔検出データを更新
	
	for (int i = 0, n = fdata->QueryNumberOfDetectedFaces(); i < n; i++) { // 検出した顔の分だけループ
		FaceData::Face *face = fdata->QueryFaceByIndex(i);

		if (face) { // 取得できなかった場合null
			FaceData::LandmarksData *lmdata = face->QueryLandmarks(); // RealSenseの特徴点データを取得

			if (lmdata) { // 取得できなかった場合null
				int numPoints = lmdata->QueryNumPoints(); // 実際に見つかった特徴点の数を取得
				if (numPoints == fconfig->landmarks.numLandmarks) { // 特徴点の数が想定していた数と一致
					lmdata->QueryPoints(landmarkPoints); // 特徴点の詳細なデータを取得

					if (landmarkPoints->confidenceImage) {
						for (int j = 0; j < numPoints; j++) {
							FaceData::LandmarkType alias = landmarkPoints[j].source.alias; // 特徴点のラベル
							int x = landmarkPoints[j].image.x; // 特徴点の座標
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

