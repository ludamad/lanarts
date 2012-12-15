/*
 * load_tileset_data.cpp:
 *  Load tile data
 */
#include <fstream>

#include "../../data/game_data.h"
#include <yaml-cpp/yaml.h>
#include "../../data/yaml_util.h"

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
	Range altfloor = floor;
	Range corridor = altfloor;
	Range altwall = wall;
	Range altcorridor = corridor;

	if (yaml_has_node(node, "corridor_tile"))
		corridor = parse_tile_range(node["corridor_tile"]);

	if (yaml_has_node(node, "alt_floor_tile"))
		altfloor = parse_tile_range(node["alt_floor_tile"]);

	if (yaml_has_node(node, "alt_wall_tile"))
		altwall = parse_tile_range(node["alt_wall_tile"]);

	if (yaml_has_node(node, "alt_corridor_tile"))
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
