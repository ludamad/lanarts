
#include <lua.hpp>
#include <fstream>

#include "data/game_data.h"
#include "data/parse.h"
#include "data/yaml_util.h"

#include "lua_api/lua_yaml.h"

#include "items/WeaponEntry.h"

#include "items/ItemEntry.h"

#include "load_stats.h"

void lapi_data_create_weapon(const LuaStackValue& table) {
	WeaponEntry* entry = new WeaponEntry;
	game_item_data.push_back(entry);
        luawrap::globals(table.luastate())["items"][entry->name] = table;
	int idx = game_item_data.size();
	entry->init(idx, table);
}
