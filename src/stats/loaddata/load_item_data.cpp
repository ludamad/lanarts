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

static _ItemEntry::id_type _parse_id_type(const YAML::Node& n) {
	std::string str = parse_str(n);
	if (str == "potion") {
		return _ItemEntry::POTION;
	} else if (str == "scroll") {
		return _ItemEntry::SCROLL;
	} else {
		LANARTS_ASSERT(false);
		return _ItemEntry::ALWAYS_KNOWN;
	}
}

_ItemEntry parse_item_type(const YAML::Node& n) {
	return _ItemEntry(
			parse_str(n["name"]),
			parse_defaulted(n, "description", std::string()),
			parse_defaulted(n, "use_message", std::string()),
			parse_defaulted(n, "radius", 11),
			parse_sprite_number(n, "spr_item"),
			parse_defaulted(n, "action_func", std::string()),
			parse_defaulted(n, "prereq_func", std::string()),
			parse_defaulted(n, "stackable", true),
			parse_defaulted(n, "shop_cost", Range()),
			yaml_has_node(n, "type") ?
					_parse_id_type(n["type"]) : _ItemEntry::ALWAYS_KNOWN);
}

void _load_item_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	_game_item_data.push_back(parse_item_type(node));
	const std::string& name = _game_item_data.back().name;
	value->table_set_yaml(L, name, node);
//	value->table_push_value(L, name);
//	lua_pushstring(L, "consumable");
//	lua_setfield(L, -2, "type");
//	lua_pop(L, 1);
}

LuaValue _load_item_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret;

	_game_item_data.clear();

	load_data_impl_template(filenames, "items", _load_item_callbackf, L, &ret);

	return ret;
}

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
