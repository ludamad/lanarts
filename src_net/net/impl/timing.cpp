#include "../timing.h"

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;

struct TimerImpl {
	ptime time;
};

static TimerImpl* timerimpl_new() {
	TimerImpl* timer = new TimerImpl();
	timer->time = microsec_clock::local_time();
	return timer;
}

static void timerimpl_start(TimerImpl* timer) {
	timer->time = microsec_clock::local_time();
}

static long timerimpl_get_microseconds(TimerImpl* timer) {
	ptime now = microsec_clock::local_time();
	time_duration duration = now - timer->time;
	return duration.total_microseconds();
}

Timer::Timer() :
		__timer(timerimpl_new()) {
}

Timer::~Timer() {
	delete __timer;
}
void Timer::start() {
	timerimpl_start(__timer);
}
long Timer::get_microseconds() {
	return timerimpl_get_microseconds(__timer);
}
