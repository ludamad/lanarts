/*
 * display.h:
 *  Display control functions, such as setting parameters, and indicating that
 *  you are done drawing.
 */

#ifndef LDRAW_DISPLAY_H_
#define LDRAW_DISPLAY_H_

#include <SDL.h>
#include <lcommon/geometry.h>

namespace ldraw {

void display_initialize(const char* window_name, const Size& draw_area_size,
		bool fullscreen = false);
void display_set_fullscreen(bool fullscreen);
bool display_is_fullscreen();
void display_set_window_region(const BBoxF& bbox);
void display_set_world_region(const BBoxF& bbox);

void display_draw_finish();
void display_draw_start();

Size screen_size();
Size display_size();
void reset_blend_func();
Size window_size();
void process_event(const SDL_Event* event);

}
        
#endif /* DISPLAY_H_ */
