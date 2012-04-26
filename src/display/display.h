/*
 * Display.h
 *
 *  Created on: Jun 8, 2011
 *      Author: 100397561
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <SDL.h>
#include <GL/glu.h>
#undef GL_GLEXT_VERSION
#include <SDL_opengl.h>
#include "../world/GameView.h"
#include "../util/draw_util.h"

struct GLImage;

void gl_set_drawing_area(int x, int y, int w, int h);
void init_sdl_gl(bool fullscreen, int w, int h);

void update_display();
void SDL_GL_diagnostics();

void gl_image_from_bytes(GLImage* img, int w, int h, char* data, int type = GL_BGRA);



void gl_draw_image(GLImage* img, int x, int y, const Colour& c = Colour(255,255,255));
inline void gl_draw_image(const GameView& view, GLImage* img, int x, int y){
	gl_draw_image(img, x - view.x, y - view.y);
}

#endif /* DISPLAY_H_ */
