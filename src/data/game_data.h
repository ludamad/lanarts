/*
 * game_data.h
 *
 *  Created on: 2011-10-27
 *      Author: 100397561
 */

#ifndef GAME_DATA_H_
#define GAME_DATA_H_

#include "../util/LuaValue.h"

#include "../world/GameSettings.h"

#include "armour_data.h"
#include "class_data.h"
#include "datafiles_data.h"
#include "dungeon_data.h"
#include "effect_data.h"
#include "enemy_data.h"
#include "item_data.h"
#include "spell_data.h"
#include "sprite_data.h"
#include "tile_data.h"
#include "tileset_data.h"
#include "weapon_data.h"

struct lua_State;

/* Contain information which guides the rest of the data loading */
GameSettings load_settings_data(const char* filename);
DataFiles load_datafiles_data(const char* filename);

void init_game_data(lua_State* L);
void init_lua_data(GameState* gs, lua_State* L);

#endif /* GAME_DATA_H_ */
