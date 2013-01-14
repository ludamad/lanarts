/*
 * window.cpp:
 *  Holds constants related to current drawing window.
 *  They are implemented as 'global getters', which use global data.
 */

#ifndef WINDOW_H_
#define WINDOW_H_

#include <lcommon/geometry.h>

static Dim __window_size;
static bool __fullscreen;

void window_set_information(Dim window_size, bool fullscreen) {
	__window_size = window_size;
	__fullscreen = fullscreen;
}

Dim window_size() {
	return __window_size;
}
bool window_fullscreen() {
	return __fullscreen;
}

#endif /* WINDOW_H_ */
