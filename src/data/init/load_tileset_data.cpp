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

static Range parse_tile_range(const YAML::Node& n) {
	Range gr;
	if (n.Type() == YAML::NodeType::Sequence) {
		std::vector<std::string> components;
		n >> components;
		gr.min = get_tile_by_name(components[0].c_str());
		gr.max = get_tile_by_name(components[1].c_str());
	} else {
		std::string s;
		n >> s;
		gr.min = get_tile_by_name(s.c_str());
		gr.max = gr.min;
	}
	return gr;
}

static TilesetEntry parse_tilesetentry(const YAML::Node& node) {
	std::string name;
	node["tileset"] >> name;

	Range floor = parse_tile_range(node["floor_tile"]);
	Range wall = parse_tile_range(node["wall_tile"]);
	Range corridor = floor;
	Range altfloor = floor;
	Range altwall = wall;
	Range altcorridor = corridor;

	if (hasnode(node, "corridor_tile"))
		corridor = parse_tile_range(node["corridor_tile"]);

	if (hasnode(node, "alt_floor_tile"))
		altfloor = parse_tile_range(node["alt_floor_tile"]);

	if (hasnode(node, "alt_wall_tile"))
		altwall = parse_tile_range(node["alt_wall_tile"]);

	if (hasnode(node, "alt_corridor_tile"))
		altcorridor = parse_tile_range(node["alt_corridor_tile"]);

	return TilesetEntry(name, floor.min, floor.max, wall.min, wall.max,
			corridor.min, corridor.max, altfloor.min, altfloor.max, altwall.min,
			altwall.max, altcorridor.min, altcorridor.max);
}

void load_tileset_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	game_tileset_data.push_back(parse_tilesetentry(node));
}

void load_tileset_data(const FilenameList& filenames) {
	game_tileset_data.clear();
	load_data_impl_template(filenames, "tilesets", load_tileset_callbackf);
}
