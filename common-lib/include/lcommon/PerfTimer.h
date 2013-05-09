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
	long max_microseconds;
	long long total_microseconds;
	double avg;
	double qvalue; // used in formula standard deviation = square root of (Q / total calls)
	MethodPerfProfile() :
			total_calls(0), max_microseconds(0), total_microseconds(0), avg(0), qvalue(0) {
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
	double average_time(const char* method);
	void print_results();
	void clear();
private:
	typedef std::map<const char*, MethodPerfProfile> MethodPerfProfileMap;
	std::map<const char*, MethodPerfProfile> perf_map;
};

#endif /* PERFTIMER_H_ */
