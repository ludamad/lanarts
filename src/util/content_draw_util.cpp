/*
 * content_draw_util.cpp:
 *  Common routines for content descriptions and drawing
 */

#include "../data/sprite_data.h"

#include "../display/display.h"

#include "../world/GameState.h"

#include "content_draw_util.h"

const char* projectile_speed_description(int speed) {
	return NULL;
}

const char* monster_speed_description(int speed) {
	return NULL;
}

void draw_stat_text(GameState *gs, int x, int y, const char *prefix,
		Colour prefix_col, int stat, Colour stat_col) {
	Pos p = gl_printf_y_centered(gs->primary_font(), prefix_col, x, y, "%s",
			prefix);
	gl_printf_y_centered(gs->primary_font(), stat_col, p.x + x, y, "%d", stat);

}

