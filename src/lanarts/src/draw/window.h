/*
 * window.h:
 *  Holds constants related to current drawing window.
 *  They are implemented as 'global getters', which use global data.
 */

#ifndef WINDOW_H_
#define WINDOW_H_

#include <lcommon/geometry.h>

void window_set_information(Dim window_size, bool fullscreen);

Dim window_size();
bool window_fullscreen();

#endif /* WINDOW_H_ */
