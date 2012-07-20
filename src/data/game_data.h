/*
 * game_data.h:
 *  Handles loading of all the various game data that is described in yaml files
 */

#ifndef GAME_DATA_H_
#define GAME_DATA_H_

#include "../lua/LuaValue.h"

#include "../gamestate/GameSettings.h"

#include "../stats/armour_data.h"
#include "../stats/class_data.h"
#include "datafilenames.h"
#include "../levelgen/dungeon_data.h"
#include "../stats/effect_data.h"
#include "../objects/enemy/enemy_data.h"
#include "../stats/item_data.h"
#include "../stats/projectile_data.h"
#include "../objects/scriptobject_data.h"
#include "../stats/spell_data.h"
#include "../display/sprite_data.h"
#include "../display/tile_data.h"
#include "../gamestate/tileset_data.h"
#include "../stats/weapon_data.h"

struct lua_State;

/* Contain information which guides the rest of the data loading */
GameSettings load_settings_data(const char* filename);
DataFiles load_datafilenames(const char* filename);

void init_game_data(lua_State* L);
void init_lua_data(GameState* gs, lua_State* L);

#endif /* GAME_DATA_H_ */
