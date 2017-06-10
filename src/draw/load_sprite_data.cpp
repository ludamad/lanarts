/*
 * load_sprite_data.cpp:
 *   Loads sprite data from
 */
#include <fstream>
#include <yaml-cpp/yaml.h>

#include <luawrap/luawrap.h>
#include <lcommon/strformat.h>

#include "lua_api/lua_yaml.h"
#include "lua_api/lua_api.h"

#include "draw/parse_drawable.h"

#include "data/game_data.h"
#include "data/parse.h"
#include "data/yaml_util.h"

using namespace std;

void load_tile_callbackf(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	TileEntry entry;
	entry.name = parse_str(node["name"]);
    entry.images = parse_image_list(node["files"]);

    game_tile_data.new_entry(entry.name, entry);
}

void load_tile_data(const FilenameList& filenames) {
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
    game_sprite_data.new_entry(entry.name, entry);

	(*value)[entry.name] = entry.sprite;
}

/*
 * Load the resources compiled by runtime/compile_images.py.
 */
static void load_compiled_resources(lua_State* L) {
    LuaValue resources = lua_api::import(L, "compiled.Resources");
    for (string id : resources["resource_id_list"].as<vector<string>>()) {
        SpriteEntry entry { id, resources[id].as<ldraw::Drawable>(), type_from_str(""), Colour() };
        game_sprite_data.new_entry(entry.name, entry);
    }
}

LuaValue load_sprite_data(lua_State* L, const FilenameList& filenames) {
	LuaValue lsprites = luawrap::ensure_table(luawrap::globals(L)["sprites"]);

	load_data_impl_template(filenames, "sprites", load_sprite_callbackf, L,
			&lsprites);

	load_compiled_resources(L);
	return lsprites;
}

