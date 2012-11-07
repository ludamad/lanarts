/*
 * display.h:
 *  Display control functions, such as setting parameters, and indicating that
 *  you are done drawing.
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <common/geometry.h>

namespace ldraw {

void display_initialize(const char* window_name, const Dim& draw_area_size,
		bool fullscreen = false);
void display_set_fullscreen(bool fullscreen);
bool display_is_fullscreen();
void display_set_drawing_region(const BBoxF& bbox);

void draw_finish();
void draw_start();

}

#endif /* DISPLAY_H_ */
