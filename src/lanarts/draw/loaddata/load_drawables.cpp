///*
// * load_sprite_data.cpp:
// *   Loads sprite data from
// */
//#include <fstream>
//#include <yaml-cpp/yaml.h>
//
//#include <draw/Drawable.h>
//
//#include <common/lua/luacpp.h>
//#include <common/strformat.h>
//
//#include "../../lua/lua_yaml.h"
//#include "../../data/game_data.h"
//#include "../../data/yaml_util.h"
//
//using namespace std;
//using namespace ldraw;
//
//std::vector<Drawable> drawables;
//
//void load_tile_callbackf(const YAML::Node& node, lua_State* L,
//		LuaValue* value) {
//	game_tile_data.push_back(
//			TileEntry(parse_str(node["name"]), parse_imgfilelist(node)));
//}
//
//void load_tile_data(const FilenameList& filenames) {
//	game_tile_data.clear();
//
//	load_data_impl_template(filenames, "tiles", load_tile_callbackf);
//
//	for (int i = 0; i < game_tile_data.size(); i++) {
//		game_tile_data[i].init();
//	}
//}
//
//static SpriteEntry::sprite_type type_from_str(const std::string& type) {
//	if (type == "directional")
//		return SpriteEntry::DIRECTIONAL;
//	return SpriteEntry::ANIMATED;
//}
//
//void load_sprite_callbackf(const YAML::Node& node, lua_State* L,
//		LuaValue* value) {
//	std::string type = parse_defaulted(node, "type", std::string());
//	SpriteEntry entry = SpriteEntry(parse_str(node["name"]),
//			parse_imgfilelist(node), type_from_str(type),
//			parse_defaulted(node, "colour", Colour()));
//	game_sprite_data.push_back(entry);
//
//	value->get(L, entry.name) = node;
//}
//
//LuaValue load_sprite_data(lua_State* L, const FilenameList& filenames) {
//	LuaValue ret;
//	ret.table_initialize(L);
//
//	game_sprite_data.clear();
//
//	load_data_impl_template(filenames, "sprites", load_sprite_callbackf, L,
//			&ret);
//
//	for (int i = 0; i < game_sprite_data.size(); i++) {
//		game_sprite_data[i].init();
//	}
//
//	return ret;
//}
//
