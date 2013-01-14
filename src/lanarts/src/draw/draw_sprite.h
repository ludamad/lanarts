/*
 * draw_sprite.h:
 *  Image drawing and other drawing related utility functions
 */

#ifndef DRAWSPRITE_H_
#define DRAWSPRITE_H_

#include <cstdlib>
#include <SDL_opengl.h>
#include <GL/glu.h>

#include <lcommon/geometry.h>
#include <ldraw/Colour.h>

#include "lanarts_defines.h"

struct GameView;
struct SpriteEntry;

void draw_sprite(sprite_id sprite, int x, int y,
		const Colour& c = Colour(255, 255, 255));
void draw_sprite(const GameView& view, sprite_id sprite, int x, int y,
		const Colour& c = Colour(255, 255, 255));
void draw_sprite(const GameView& view, sprite_id sprite, int x, int y,
		float dx, float dy, float frame, const Colour& c = Colour(255, 255, 255));

#endif /* DRAWSPRITE_H_ */
