#include "../timing.h"

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;

struct TimerImpl {
	ptime time;
};

TimerImpl* timerimpl_new() {
	TimerImpl* timer = new TimerImpl();
	timer->time = microsec_clock::local_time();
	return timer;
}

void timerimpl_start(TimerImpl* timer) {
	timer->time = microsec_clock::local_time();
}

long timerimpl_get_microseconds(TimerImpl* timer) {
	ptime now = microsec_clock::local_time();
	time_duration duration = now - timer->time;
	return duration.total_microseconds();
}

void timerimpl_free(TimerImpl* timer) {
	delete timer;
}

