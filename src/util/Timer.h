/*
 * Timer.h:
 *  Simple timing utility
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <SDL.h>

class Timer {
public:
	Timer();
	void start();
	unsigned long get_microseconds();
private:
	long long microseconds_since_epoch;
};

#endif /* TIMER_H_ */
