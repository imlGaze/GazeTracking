#include <chrono>
#include "Stopwatch.h"

using namespace std::chrono;

nanoseconds Stopwatch::now() {
	return duration_cast<nanoseconds>(
		system_clock::now().time_since_epoch()
		);
}

void Stopwatch::start() {
	nanoseconds time = now();
	first = time;
	prev = time;
}

long Stopwatch::lap() {
	nanoseconds time = now();
	nanoseconds duration = time - prev;
	prev = time;

	return duration.count();
}

long Stopwatch::stop() {
	nanoseconds time = now();
	nanoseconds duration = time - first;

	first = nanoseconds(0);
	prev = nanoseconds(0);
	
	return duration.count();
}
