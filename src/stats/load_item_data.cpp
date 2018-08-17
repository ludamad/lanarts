/*
 * load_item_data.cpp:
 *  Implements item loading routines from YAML datafiles
 */

#include <lua.hpp>

#include <fstream>

#include <yaml-cpp/yaml.h>

#include "data/game_data.h"
#include "data/parse.h"
#include "data/yaml_util.h"

#include "lua_api/lua_yaml.h"

#include "items/ItemEntry.h"

using namespace std;

void lapi_data_create_item(const LuaStackValue& table) {
	ItemEntry* entry = new ItemEntry;
	entry->init(game_item_data.size(), table);
        table["__method"] = "item_create"; 

	game_item_data.new_entry(entry->name, entry, table);
}

