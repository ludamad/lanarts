#include "lcommon_defines.h"

#include "PerfTimer.h"

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

void perf_print_results() {
	__global_timer.print_results();
}

