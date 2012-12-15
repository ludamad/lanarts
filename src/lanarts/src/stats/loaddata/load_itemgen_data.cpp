/*
 * load_item_data.cpp:
 *  Implements item loading routines from YAML datafiles
 */

#include <lua.hpp>

#include <fstream>

#include <yaml-cpp/yaml.h>

#include "data/game_data.h"
#include "data/parse_with_defaults.h"
#include "data/yaml_util.h"

#include "lua/lua_yaml.h"

#include "../items/ItemEntry.h"

#include "../itemgen_data.h"

using namespace std;

static ItemGenChance parse_item_chance(const YAML::Node& n) {
	ItemGenChance igc;
	string itemname;
	n["item"] >> itemname;
	igc.itemtype = get_item_by_name(itemname.c_str());
	n["chance"] >> igc.genchance;
	igc.quantity = parse_defaulted(n, "quantity", Range(1, 1));
	return igc;
}
ItemGenList parse_itemgenlist(const YAML::Node& n) {
	ItemGenList igs;
	igs.name = parse_str(n["name"]);
	igs.items = parse_named_with_defaults(n["items"], "item",
			&parse_item_chance);
	return igs;
}
static void load_itemlist(const YAML::Node& node, lua_State* L,
		LuaValue* value) {
	ItemGenList igl = parse_itemgenlist(node);
	game_itemgenlist_data.push_back(igl);

	(*value)[igl.name] = node;
}

LuaValue load_itemgenlist_data(lua_State* L, const FilenameList& filenames) {
	LuaValue ret(L);
	ret.newtable();

	game_itemgenlist_data.clear();

	load_data_impl_template(filenames, "itemgen_lists", load_itemlist,
			L, &ret);

	return ret;
}
