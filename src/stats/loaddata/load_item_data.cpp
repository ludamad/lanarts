/*
 * load_item_data.cpp:
 *  Implements item loading routines from YAML datafiles
 */

#include <fstream>

#include <yaml-cpp/yaml.h>

#include "../item_data.h"
#include "../../data/game_data.h"

#include "../../data/yaml_util.h"

extern "C" {
#include <lua/lua.h>
}

using namespace std;

static ItemEntry::id_type parse_id_type(const YAML::Node& n) {
	std::string str = parse_str(n);
	if (str == "potion") {
		return ItemEntry::POTION;
	} else if (str == "scroll") {
		return ItemEntry::SCROLL;
	} else {
		LANARTS_ASSERT(false);
		return ItemEntry::ALWAYS_KNOWN;
	}
}

ItemEntry parse_item_type(const YAML::Node& n) {
	return ItemEntry(
			parse_str(n["name"]),
			parse_defaulted(n, "description", std::string()),
			parse_defaulted(n, "use_message", std::string()),
			parse_defaulted(n, "radius", 11),
			parse_sprite_number(n, "sprite"),
			parse_defaulted(n, "action_func", std::string()),
			parse_defaulted(n, "prereq_func", std::string()),
			parse_defaulted(n, "stackable", true),
			yaml_has_node(n, "type") ?
					parse_id_type(n["type"]) : ItemEntry::ALWAYS_KNOWN);
}

void load_item_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	game_item_data.push_back(parse_item_type(node));
	const std::string& name = game_item_data.back().name;
	value->table_set_yaml(L, name, node);
//	value->table_push_value(L, name);
//	lua_pushstring(L, "consumable");
//	lua_setfield(L, -2, "type");
//	lua_pop(L, 1);
}

LuaValue load_item_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret;

	game_item_data.clear();

	load_data_impl_template(filenames, "items", load_item_callbackf, L, &ret);

	return ret;
}
