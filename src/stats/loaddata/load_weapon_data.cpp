#include <lua.hpp>
#include <fstream>

#include "data/game_data.h"
#include "data/parse.h"
#include "data/yaml_util.h"

#include "lua_api/lua_yaml.h"

#include "../items/WeaponEntry.h"

#include "../items/ItemEntry.h"

#include "load_stats.h"

static LuaValue* weapon_table;
static void lapi_data_create_weapon(const LuaStackValue& table) {
	WeaponEntry* entry = new WeaponEntry;
	game_item_data.push_back(entry);

	int idx = game_item_data.size();
	entry->init(idx, table);

	(*weapon_table)[entry->name] = table;
}

void load_weapon_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemtable) {
	weapon_table = itemtable;

	LuaValue data = luawrap::ensure_table(luawrap::globals(L)["Data"]);
	data["weapon_create"].bind_function(lapi_data_create_weapon);
	luawrap::dofile(L, "items/Weapons.lua");
}
