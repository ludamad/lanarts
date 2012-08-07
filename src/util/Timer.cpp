/*
 * Timer.cpp:
 *  Simple timing utility
 *  Here be dragons.
 */

#include "Timer.h"

#include <SDL.h>

#ifdef __unix__
#include <sys/time.h>
#endif

#ifdef _WIN32
#include <windows.h>
const long long DELTA_EPOCH_IN_MICROSECS= 11644473600000000;

#ifndef _TIMEVAL_DEFINED
struct timeval {
	int tv_sec; /* seconds */
	int tv_usec; /* microseconds */
};
#endif

int gettimeofday(struct timeval *tv, void* notused) {
	FILETIME ft;
	long long tmpres = 0;

	memset(&ft, 0, sizeof(ft));

	GetSystemTimeAsFileTime(&ft);

	tmpres = ft.dwHighDateTime;
	tmpres <<= 32;
	tmpres |= ft.dwLowDateTime;

	/*converting file time to unix epoch*/
	tmpres /= 10; /*convert into microseconds*/
	tmpres -= DELTA_EPOCH_IN_MICROSECS;
	tv->tv_sec = (int)(tmpres*0.000001);
	tv->tv_usec =(tmpres%1000000);

	return 0;
}
#endif

Timer::Timer() {
	start();
}

void Timer::start() {
#if defined(__unix__) || defined(_WIN32)
	timeval timev;
	gettimeofday(&timev, NULL);
	microseconds_since_epoch = timev.tv_sec * 1000 * 1000 + timev.tv_usec;

#else
	microseconds_since_epoch = SDL_GetTicks() * 1000;
#endif
}

unsigned long Timer::get_microseconds() {
	Timer now;
	return now.microseconds_since_epoch - microseconds_since_epoch;
}
