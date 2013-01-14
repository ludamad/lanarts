/*
 * load_sprite_data.cpp:
 *   Loads sprite data from
 */
#include <fstream>
#include <yaml-cpp/yaml.h>

#include <luawrap/luawrap.h>
#include <lcommon/strformat.h>

#include "lua/lua_yaml.h"

#include "draw/parse_drawable.h"

#include "data/game_data.h"
#include "data/parse.h"
#include "data/yaml_util.h"

using namespace std;

/* Either given as a list of filenames or a base name, extension,
 * and number of sequentially numbered files */
static FilenameList parse_imgfilelist(const YAML::Node& node) {
	int seq = parse_defaulted(node, "variations", 0);
	bool is_seq = seq > 0;

	FilenameList filenames;

	if (is_seq) {
		std::string tilefile = parse_str(node["file"]);
		std::string ext = parse_str(node["extension"]);
		for (int i = 0; i < seq; i++) {
			filenames.push_back(
					format("%s%d%s", tilefile.c_str(), i, ext.c_str()));
		}
	} else {
		node["file"] >> filenames;
	}
	return filenames;
}


void load_tile_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	TileEntry entry;
	entry.name = parse_str(node["name"]);
	entry.images =parse_image_list(node["files"]);

	game_tile_data.push_back(entry);
}

void load_tile_data(const FilenameList& filenames) {
	game_tile_data.clear();

	load_data_impl_template(filenames, "tiles", load_tile_callbackf);
}

static SpriteEntry::sprite_type type_from_str(const std::string& type) {
	if (type == "directional")
		return SpriteEntry::DIRECTIONAL;
	return SpriteEntry::ANIMATED;
}

void load_sprite_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	std::string type = parse_defaulted(node, "type", std::string());
	SpriteEntry entry = SpriteEntry(parse_str(node["name"]),
			parse_drawable(node), type_from_str(type),
			parse_defaulted(node, "colour", Colour()));
	game_sprite_data.push_back(entry);

	(*value)[entry.name] = entry.sprite;
}
LuaValue load_sprite_data(lua_State* L, const FilenameList& filenames) {
	LuaValue lsprites = luawrap::globals(L)["sprites"].ensure_table();

	game_sprite_data.clear();

	load_data_impl_template(filenames, "sprites", load_sprite_callbackf, L,
			&lsprites);


	return lsprites;
}

