/*
 * timing.h:
 *  Simple high resolution timing.
 */
#ifndef __TIMING_H_
#define __TIMING_H_

//Take care not to introduce boost dependencies, even if we sacrifice performance!

struct TimerImpl;

TimerImpl* timerimpl_new();
void timerimpl_start(TimerImpl* timer);
long timerimpl_get_microseconds(TimerImpl* timer);
void timerimpl_free(TimerImpl* timer);

class Timer {
public:
	Timer() :
			__timer(timerimpl_new()) {
	}
	~Timer() {
		timerimpl_free(__timer);
	}
	void start(){
		timerimpl_start(__timer);
	}
	long get_microseconds(){
		return timerimpl_get_microseconds(__timer);
	}
private:
	TimerImpl* __timer;
};

#endif
