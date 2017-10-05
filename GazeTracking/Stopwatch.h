#pragma once

#include<vector>
#include<chrono>

using namespace std::chrono;
using std::vector;

class Stopwatch {
	nanoseconds first;
	nanoseconds prev;

public:
	nanoseconds now();

	void start();
	long lap();
	long stop();

};