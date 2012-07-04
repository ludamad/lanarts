/*
 * content_draw_util.h:
 *  Common routines for content descriptions and drawing
 */

#ifndef CONTENT_DRAW_UTIL_H_
#define CONTENT_DRAW_UTIL_H_

#include "game_basic_structs.h"

class GameState;

const char* projectile_speed_description(int speed);
const char* monster_speed_description(int speed);

void draw_projectile_info();

void draw_stat_text(GameState* gs, int x, int y, const char* prefix,
		Colour prefix_col, int stat, Colour stat_col);
#endif /* CONTENT_DRAW_UTIL_H_ */
