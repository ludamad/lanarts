#include "util/PerfTimer.h"
#include "lanarts_defines.h"

Colour Colour::mute_colour(float val) const {
	return inverse().scale(val).inverse();
}

Colour Colour::with_alpha(int alpha) const {
	return Colour(r, g, b, alpha);
}

Colour Colour::scale(float val) const {
	return Colour(r * val, g * val, b * val, a * val);
}

Colour Colour::inverse() const {
	return Colour(255 - r, 255 - g, 255 - b, 255 - a);
}

Colour Colour::mult_alpha(float alpha) const {
	return Colour(r, g, b, a * alpha);
}

Colour Colour::multiply(const Colour & o) const {
	return Colour(r * (o.r + 1) / 256, g * (o.g + 1) / 256, b * (o.b + 1) / 256,
			a * (o.a + 1) / 256);
}

static PerfTimer __global_timer;

void perf_timer_begin(const char* funcname) {
	__global_timer.begin(funcname);
}

void perf_timer_end(const char* funcname) {
	__global_timer.end(funcname);
}

void perf_print_results() {
	__global_timer.print_results();
}

