/*
 * components_draw.h:
 *  Helper methods for common interface components
 */

#ifndef COMPONENTS_DRAW_H_
#define COMPONENTS_DRAW_H_

#include "../lanarts_defines.h"

class GameState;

void draw_setting_box(GameState* gs, const BBox& bbox,
		const Colour& bbox_col, sprite_id sprite, const Colour& sprite_col,
		const char* text, const Colour& text_col);
void draw_option_box(GameState* gs, const BBox& bbox, bool option_set,
		sprite_id sprite, const char* text);
void draw_speed_box(GameState* gs, const BBox& bbox);

#endif /* COMPONENTS_DRAW_H_ */
