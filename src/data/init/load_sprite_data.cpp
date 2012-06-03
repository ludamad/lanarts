/*
 * load_sprite_data.cpp
 *
 *  Created on: Mar 25, 2012
 *      Author: 100397561
 */
#include <fstream>

#include "../game_data.h"
#include <yaml-cpp/yaml.h>
#include "yaml_util.h"

using namespace std;

void load_tile_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	int seq = parse_defaulted(node, "variations", 0);
	bool is_seq = seq > 0;

	FilenameList filenames;
	std::string tilefile = parse_str(node["file"]);

	if (is_seq) {
		for (int i = 0; i < seq; i++) {
			char number[32];
			snprintf(number, 32, "%d", i);
			filenames.push_back(
					tilefile + number + parse_str(node["extension"]));
		}
	} else {
		filenames.push_back(tilefile);
	}

	game_tile_data.push_back(TileEntry(parse_str(node["name"]), filenames));
}

void load_tile_data(const FilenameList& filenames) {
	game_tile_data.clear();

	load_data_impl_template(filenames, "tiles", load_tile_callbackf);

	for (int i = 0; i < game_tile_data.size(); i++) {
		game_tile_data[i].init();
	}
}

void load_sprite_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	FilenameList files(1);
	node["file"] >> files[0];

	SpriteEntry entry(parse_str(node["name"]), files);
	game_sprite_data.push_back(entry);
	value->table_set_yaml(L, game_sprite_data.back().name.c_str(), &node);
}
LuaValue load_sprite_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret;

	game_sprite_data.clear();

	load_data_impl_template(filenames, "sprites", load_sprite_callbackf, L,
			&ret);

	for (int i = 0; i < game_sprite_data.size(); i++) {
		game_sprite_data[i].init();
	}

	return ret;
}

