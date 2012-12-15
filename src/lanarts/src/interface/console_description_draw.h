/*
 * console_description_draw.h:
 *  Common routines for content descriptions and drawing
 */

#ifndef CONSOLE_DESCRIPTION_DRAW_H_
#define CONSOLE_DESCRIPTION_DRAW_H_

#include "lanarts_defines.h"

struct Item;
struct ItemEntry;
struct EnemyEntry;
struct SpellEntry;
class GameState;

const char* projectile_speed_description(int speed);
const char* monster_speed_description(int speed);

void draw_spell_icon_and_name(GameState* gs, SpellEntry& spl_entry, Colour col,
		int x, int y);

int draw_icon_and_name(GameState* gs, BaseDataEntry& entry, Colour col, int x,
		int y, int xoffset = TILE_SIZE * 1.25, int yoffset = TILE_SIZE / 2);

void draw_console_spell_description(GameState* gs, SpellEntry& spl_entry);
void draw_console_item_description(GameState* gs, const Item& item,
		ItemEntry& ientry);
void draw_console_enemy_description(GameState* gs, EnemyEntry& eentry);

#endif /* CONTENT_DRAW_UTIL_H_ */
