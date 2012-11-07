/*
 * PerfTimer.h:
 *  Provides timing information on a per-method basis
 */

#ifndef PERFTIMER_H_
#define PERFTIMER_H_

#include <map>
#include "Timer.h"

struct MethodPerfProfile {
	Timer timer;
	int total_calls;
	long long total_microseconds;
	MethodPerfProfile(int total_calls = 0, long long total_microseconds = 0) :
			total_calls(total_calls), total_microseconds(total_microseconds) {
	}
	void begin_timer();
	void end_timer();
};

class PerfTimer {
public:
	// Call start(), stop() at the start and end of a method call
	// Note: Does not work with recursive methods
	void begin(const char* method);
	void end(const char* method);
	void print_results();
	void clear();
private:
	typedef std::map<const char*, MethodPerfProfile> MethodPerfProfileMap;
	std::map<const char*, MethodPerfProfile> perf_map;
};


void perf_timer_begin(const char* funcname);
void perf_timer_end(const char* funcname);
void perf_timer_clear();
void perf_print_results();

#endif /* PERFTIMER_H_ */
