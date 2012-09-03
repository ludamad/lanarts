/*
 * PerfTimer.cpp:
 *  Provides timing information on a per-method basis
 */

#include "PerfTimer.h"

void MethodPerfProfile::begin_timer() {
	timer.start();
}

void MethodPerfProfile::end_timer() {
	total_calls++;
	total_microseconds += timer.get_microseconds();
}

void PerfTimer::begin(const char* method) {
	perf_map[method].begin_timer();
}

void PerfTimer::end(const char* method) {
	perf_map[method].end_timer();
}

void PerfTimer::print_results() {
	MethodPerfProfileMap::iterator it = perf_map.begin();
	for (; it != perf_map.end(); ++it) {
		MethodPerfProfile& mpp = it->second;
		float total = mpp.total_microseconds / 1000.0f;
		float avg = total / mpp.total_calls;
		printf(
				"%s():\n\tAVG %.4fms\tTOTAL\t %.4fms\tCALLS\t %d\n",
				it->first, avg, total, mpp.total_calls);
	}
}
