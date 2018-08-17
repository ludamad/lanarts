
#include <lua.hpp>
#include <fstream>

#include "data/game_data.h"
#include "data/parse.h"
#include "data/yaml_util.h"

#include "lua_api/lua_yaml.h"
#include "lua_api/lua_api.h"

#include "items/WeaponEntry.h"

#include "items/ItemEntry.h"

#include "load_stats.h"

void lapi_data_create_weapon(const LuaStackValue& table) {
	WeaponEntry* entry = new WeaponEntry;
	int idx = game_item_data.size();
        table["__method"] = "weapon_create"; 
	entry->init(idx, table);
	game_item_data.new_entry(entry->name, entry, table);
}
