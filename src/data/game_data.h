/*
 * game_data.h:
 *  Handles loading of all the various game data that is described in yaml files
 */

#ifndef GAME_DATA_H_
#define GAME_DATA_H_

#include "../display/sprite_data.h"

#include "../display/tile_data.h"

#include "../gamestate/GameSettings.h"
#include "../gamestate/tileset_data.h"
#include "../levelgen/dungeon_data.h"
#include "../lua/LuaValue.h"
#include "../objects/enemy/enemy_data.h"
#include "../objects/scriptobject_data.h"

#include "../stats/items/EquipmentEntry.h"
#include "../stats/items/ProjectileEntry.h"
#include "../stats/items/WeaponEntry.h"

#include "../stats/class_data.h"
#include "../stats/effect_data.h"
#include "../stats/items/ItemEntry.h"
#include "../stats/itemgen_data.h"
#include "../stats/SpellEntry.h"

#include "datafilenames.h"

struct lua_State;
class LuaValue;

/* Contain information which guides the rest of the data loading */
GameSettings load_settings_data(const char* filename, lua_State* L = NULL,
		LuaValue* lua_settings = NULL);
DataFiles load_datafilenames(const char* filename);

GameSettings init_game_data(lua_State* L);
void init_lua_data(GameState* gs, lua_State* L);

#endif /* GAME_DATA_H_ */
