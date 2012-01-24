/*
 * Display.h
 *
 *  Created on: Jun 8, 2011
 *      Author: 100397561
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_
#include "../world/GameView.h"

struct GLImage;
int power_of_two(int input);

void gl_set_drawing_area(int x, int y, int w, int h);
void init_sdl_gl(bool fullscreen, int w, int h);
void image_display(GLImage* img, int x, int y);
inline void image_display(const GameView& view, GLImage* img, int x, int y){
	image_display(img, x - view.x, y - view.y);
}
void image_display_parts(GLImage* img, int x, int y, int sub_parts, char* flags);
void update_display();
void SDL_GL_diagnostics();

void gl_image_from_bytes(GLImage* img, int w, int h, char* data);

#endif /* DISPLAY_H_ */
