#pragma once
#include<opencv2\opencv.hpp>

using cv::Size;

const int IMAGE_COUNT = 30;
const int IMAGE_WIDTH = 640;
const int IMAGE_HEIGHT = 480;
const Size IMAGE_SIZE = Size(IMAGE_WIDTH, IMAGE_HEIGHT);
const int CHESS_SIZE = 20 / 2;

const int CHESS_COLS = 10;
const int CHESS_ROWS = 7;
const Size CHESS_PATTERN = Size(CHESS_COLS, CHESS_ROWS);

const bool CALIB_SKIP_CHECK = 1;
