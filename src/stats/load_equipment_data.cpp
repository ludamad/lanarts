#include <fstream>

#include <lua.hpp>

#include "lua_api/lua_yaml.h"
#include "lua_api/lua_api.h"
#include "data/game_data.h"
#include "data/yaml_util.h"

#include "items/ItemEntry.h"

#include "load_stats.h"

using namespace std;

void lapi_data_create_equipment(const LuaStackValue& table) {
	auto* entry = new EquipmentEntry;
	size_t idx = game_item_data.size();
        table["__method"] = "equipment_create"; 
	entry->init(idx, table);
	game_item_data.new_entry(entry->name, entry, table);
}
