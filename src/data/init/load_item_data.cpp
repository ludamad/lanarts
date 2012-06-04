/*
 * load_settings_data.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: 100397561
 */

#include <fstream>

#include "../game_data.h"
#include <yaml-cpp/yaml.h>
#include "yaml_util.h"
#include "../item_data.h"

using namespace std;

ItemEntry parse_item_type(const YAML::Node& n){
	return ItemEntry(
			parse_str(n["name"]),
			parse_defaulted(n,"radius", 11),
			parse_sprite_number(n, "sprite"),
			parse_defaulted(n, "action_func", std::string()),
			parse_defaulted(n, "prereq_func", std::string()),
			parse_defaulted(n, "stackable", 1)
		);
}

void load_item_callbackf(const YAML::Node& node, lua_State* L, LuaValue* value){
	game_item_data.push_back(parse_item_type(node));
	value->table_set_yaml(L, game_item_data.back().name, &node);
}
LuaValue load_item_data(lua_State* L, const FilenameList& filenames){
	LuaValue ret;

	game_item_data.clear();

	load_data_impl_template(filenames, "items", load_item_callbackf, L,
			&ret);

	return ret;
}
