/*
 * game_data.h:
 *  Handles loading of all the various game data that is described in yaml files
 */

#ifndef GAME_DATA_H_
#define GAME_DATA_H_

#include <luawrap/LuaValue.h>

#include "draw/SpriteEntry.h"

#include "draw/TileEntry.h"

#include "gamestate/GameSettings.h"
#include "gamestate/tileset_data.h"
#include "objects/EnemyEntry.h"

#include "stats/items/EquipmentEntry.h"
#include "stats/items/ProjectileEntry.h"
#include "stats/items/WeaponEntry.h"

#include "stats/ClassEntry.h"
#include "stats/effect_data.h"
#include "stats/items/ItemEntry.h"
#include "stats/SpellEntry.h"

#include "datafilenames.h"

struct lua_State;
class LuaValue;

/* Contain information which guides the rest of the data loading */
bool load_settings_data(GameSettings& settings, const char* filename);
void save_settings_data(GameSettings& settings, const char* filename);
DataFiles load_datafilenames(const char* filename);

void init_game_data(GameSettings& settings, lua_State* L);

#endif /* GAME_DATA_H_ */
