/*
 * console_description_draw.h:
 *  Common routines for content descriptions and drawing
 */

#ifndef CONSOLE_DESCRIPTION_DRAW_H_
#define CONSOLE_DESCRIPTION_DRAW_H_

#include "../lanarts_defines.h"

struct _Item;
struct _ItemEntry;
struct EnemyEntry;
struct SpellEntry;
class GameState;

const char* projectile_speed_description(int speed);
const char* monster_speed_description(int speed);

void draw_spell_icon_and_name(GameState* gs, SpellEntry& spl_entry, Colour col,
		int x, int y);

void draw_item_icon_and_name(GameState* gs, _ItemEntry& ientry, Colour col,
		int x, int y);

void draw_console_spell_description(GameState* gs, SpellEntry& spl_entry);
void draw_console_item_description(GameState* gs, const _Item& item);
void draw_console_enemy_description(GameState* gs, EnemyEntry& eentry);

#endif /* CONTENT_DRAW_UTIL_H_ */
