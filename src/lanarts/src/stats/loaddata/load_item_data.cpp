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

#include "../items/ItemEntry.h"

using namespace std;
void parse_item_entry(lua_State* L, const YAML::Node& n, ItemEntry& entry) {
	entry.name = parse_str(n["name"]);
	entry.description = parse_optional(n, "description", std::string());
	entry.shop_cost = parse_optional(n, "shop_cost", Range());

	entry.use_action.success_message = parse_optional(n, "use_message",
			std::string());
	entry.use_action.failure_message = parse_optional(n, "cant_use_message",
			std::string());

	entry.use_action.action_func = parse_luaexpr(L, n, "action_func");
	entry.use_action.prereq_func =  parse_luaexpr(L, n, "prereq_func");
	entry.item_sprite = parse_sprite_number(n, "spr_item");

	entry.stackable = parse_optional(n, "stackable", true);
	parse_optional(n, "shop_cost", Range());
}

static void load_item_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	ItemEntry* entry = new ItemEntry;
	parse_item_entry(L, node, *entry);
	game_item_data.push_back(entry);

	(*value)[entry->name] = node;
}

static LuaValue* item_table;
static void lapi_data_create_item(const LuaStackValue& table) {

	ItemEntry* entry = new ItemEntry;
	game_item_data.push_back(entry);

	(*item_table)[entry->name] = table;

	int idx = game_item_data.size();
	entry->init(idx, table);
	printf("Loaded %s successfully!\n", entry->name.c_str());
	game_item_data.push_back(entry);
}

LuaValue load_item_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret(L);
	ret.newtable();
	item_table = &ret;

	clear_item_data(game_item_data);

	LuaValue data = luawrap::ensure_table(luawrap::globals(L)["Data"]);
	data["item_create"].bind_function(lapi_data_create_item);
	luawrap::dofile(L, "res/items/items.lua");

	return ret;
}
