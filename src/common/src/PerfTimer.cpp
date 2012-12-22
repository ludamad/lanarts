/*
 * PerfTimer.cpp:
 *  Provides timing information on a per-method basis
 */

#include <cmath>
#include <algorithm>
#include "PerfTimer.h"

void MethodPerfProfile::begin_timer() {
	timer.start();
}

void MethodPerfProfile::end_timer() {
	long timemicro = timer.get_microseconds();

	// Note, converting to float can be pricey
	double ftime = timemicro;
	double ftotal = total_microseconds;
	double avg1 = ftotal / total_calls;
	double avg2 = (ftotal + timemicro) / (total_calls + 1);

	total_calls++;
	max_microseconds = std::max(max_microseconds, timemicro);
	total_microseconds += timemicro;
	qvalue += (ftime - avg1) * (ftime - avg2);

}

void PerfTimer::begin(const char* method) {
	perf_map[method].begin_timer();
}

void PerfTimer::end(const char* method) {
	perf_map[method].end_timer();
}

void PerfTimer::print_results() {
	printf("**** START PERFORMANCE STATS ****\n");
	MethodPerfProfileMap::iterator it = perf_map.begin();
	for (; it != perf_map.end(); ++it) {
		MethodPerfProfile& mpp = it->second;
		float total = mpp.total_microseconds / 1000.0f;
		float max = mpp.max_microseconds / 1000.0f;
		float avg = total / mpp.total_calls;
		float stddev = sqrt(mpp.qvalue / mpp.total_calls) / 1000.0f;
		printf("%s:\n\tAVG %.4fms\tTOTAL\t %.4fms\tCALLS\t %d\tMAX %.4fms\tSTDDEV %.4fms\n", it->first,
				avg, total, mpp.total_calls, max, stddev);
	}
	printf("**** END PERFORMANCE STATS ****\n");
}

double PerfTimer::average_time(const char* method) {
	MethodPerfProfile& mpp = perf_map[method];
	float total = mpp.total_microseconds / 1000.0f;
	return total / mpp.total_calls;
}

void PerfTimer::clear() {
	perf_map.clear();
}

static PerfTimer __global_timer;

void perf_timer_begin(const char* funcname) {
	__global_timer.begin(funcname);
}

void perf_timer_clear() {
	__global_timer.clear();
}

void perf_timer_end(const char* funcname) {
	__global_timer.end(funcname);
}

double perf_timer_average_time(const char* funcname) {
	return __global_timer.average_time(funcname);
}

void perf_print_results() {
	__global_timer.print_results();
}

