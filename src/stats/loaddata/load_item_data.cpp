/*
 * load_item_data.cpp:
 *  Implements item loading routines from YAML datafiles
 */

extern "C" {
#include <lua/lua.h>
}

#include <fstream>

#include <yaml-cpp/yaml.h>

#include "../../data/game_data.h"
#include "../../data/yaml_util.h"

#include "../item_data.h"

#include "../items/ItemEntry.h"

using namespace std;
void parse_item_entry(const YAML::Node& n, ItemEntry& entry) {
	entry.name = parse_str(n["name"]);
	entry.description = parse_defaulted(n, "description", std::string());
	entry.shop_cost = parse_defaulted(n, "shop_cost", Range());

	entry.use_action.success_message = parse_defaulted(n, "use_message",
			std::string());
	entry.use_action.success_message = parse_defaulted(n, "cant_use_message",
			std::string());
	entry.use_action.action_func = parse_defaulted(n, "action_func",
			std::string());

	entry.item_sprite = parse_sprite_number(n, "spr_item");
	parse_defaulted(n, "prereq_func", std::string());
	entry.stackable = parse_defaulted(n, "stackable", true);
	parse_defaulted(n, "shop_cost", Range());
}

static void load_item_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	ItemEntry* entry = new ItemEntry;
	parse_item_entry(node, *entry);
	game_item_data.push_back(entry);
	value->table_set_yaml(L, entry->name, node);
}

LuaValue load_item_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret;

	clear_item_data(game_item_data);

	load_data_impl_template(filenames, "items", load_item_callbackf, L, &ret);

	return ret;
}
