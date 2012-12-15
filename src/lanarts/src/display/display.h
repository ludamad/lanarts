/*
 * display.h:
 *  Image drawing and other drawing related utility functions
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <cstdlib>
#include <SDL_opengl.h>
#include <GL/glu.h>

#include <lcommon/geometry.h>
#include <ldraw/Colour.h>

#include "lanarts_defines.h"

struct GLimage;
struct font_data;
struct GameView;
struct SpriteEntry;

void gl_subimage_from_bytes(GLimage& img, const BBox& region, char* data,
		int type = GL_BGRA);
void gl_image_from_bytes(GLimage& img, int w, int h, char* data, int type =
		GL_BGRA);

inline void gl_init_image(GLimage& img, int w, int h, int type = GL_BGRA) {
	gl_image_from_bytes(img, w, h, NULL, type);
}


void gl_draw_image(GLimage& img, int x, int y,
		const Colour& c = Colour(255, 255, 255));
void gl_draw_image_section(GLimage& img, const BBox& section, int x, int y,
		const Colour& c = Colour(255, 255, 255));
void gl_draw_image(const GameView& view, GLimage& img, int x, int y,
		const Colour& c = Colour(255, 255, 255));
void gl_draw_sprite(sprite_id sprite, int x, int y,
		const Colour& c = Colour(255, 255, 255));
void gl_draw_sprite(const GameView& view, sprite_id sprite, int x, int y,
		const Colour& c = Colour(255, 255, 255));
void gl_draw_sprite(const GameView& view, sprite_id sprite, int x, int y,
		float dx, float dy, int steps, const Colour& c = Colour(255, 255, 255));

#endif /* DISPLAY_H_ */
