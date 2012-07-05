/*
 * content_draw_util.h:
 *  Common routines for content descriptions and drawing
 */

#ifndef CONTENT_DRAW_UTIL_H_
#define CONTENT_DRAW_UTIL_H_

#include "game_basic_structs.h"

struct Item;
struct SpellEntry;
class GameState;

const char* projectile_speed_description(int speed);
const char* monster_speed_description(int speed);

void draw_projectile_info();

void draw_stat_text(GameState* gs, int x, int y, const char* prefix,
		Colour prefix_col, int stat, Colour stat_col);

void draw_spell_icon_and_name(GameState* gs, SpellEntry& spl_entry, Colour col, int x,
		int y);
void draw_item_icon_and_name(GameState* gs, ItemEntry& ientry, Colour col, int x,
		int y);
void draw_console_spell_description(GameState* gs, SpellEntry& spl_entry);
void draw_console_item_description(GameState* gs, const Item& item);

#endif /* CONTENT_DRAW_UTIL_H_ */
