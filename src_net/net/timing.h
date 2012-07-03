/*
 * timing.h:
 *  Simple high resolution timing.
 */
#ifndef __TIMING_H_
#define __TIMING_H_

//Take care not to introduce boost dependencies, even if we sacrifice performance!

struct TimerImpl;

class Timer {
public:
	Timer();
	~Timer();
	void start();
	long get_microseconds();
private:
	TimerImpl* __timer;
};

#endif
