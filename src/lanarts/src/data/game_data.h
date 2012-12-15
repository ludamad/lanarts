/*
 * game_data.h:
 *  Handles loading of all the various game data that is described in yaml files
 */

#ifndef GAME_DATA_H_
#define GAME_DATA_H_

#include <luawrap/LuaValue.h>

#include "../display/sprite_data.h"

#include "../display/tile_data.h"

#include "../gamestate/GameSettings.h"
#include "../gamestate/tileset_data.h"
#include "../levelgen/dungeon_data.h"
#include "../objects/enemy/EnemyEntry.h"
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
void load_settings_data(GameSettings& settings, const char* filename);
void save_settings_data(GameSettings& settings, const char* filename);
DataFiles load_datafilenames(const char* filename);

void init_game_data(GameSettings& settings, lua_State* L);
void init_lua_data(GameState* gs, lua_State* L);

#endif /* GAME_DATA_H_ */
