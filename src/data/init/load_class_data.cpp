/*
 * load_dungeon_data.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: 100397561
 */


#include <fstream>

#include "../game_data.h"
#include <yaml-cpp/yaml.h>
#include "yaml_util.h"
#include "../../gamestats/Stats.h"
#include "../enemy_data.h"
#include "../class_data.h"

using namespace std;

ClassType parse_class(const YAML::Node& n){
	Attack melee(true, 10, 25, 40);
	Attack ranged(true, 8, 400, 40, get_sprite_by_name("fire bolt"), 7);
	ranged.isprojectile = true;
	vector<Attack> attacks;
	attacks.push_back(melee);
	attacks.push_back(ranged);
	const YAML::Node& gainperlevel = n["gain_per_level"];
	int hp, mp, mag, str, def;
	float hpregen, mpregen;
	gainperlevel["hp"] >> hp;
	gainperlevel["mp"] >> mp;
	gainperlevel["strength"] >> str;
	gainperlevel["magic"] >> mag;
	gainperlevel["defence"] >> def;
	gainperlevel["mpregen"] >> mpregen;
	gainperlevel["hpregen"] >> hpregen;

	return ClassType(
			parse_str(n["name"]),
			parse_stats(n["start_stats"], attacks),
			hp, mp,
			str, def, mag,
			hpregen, mpregen
			);
}

void load_class_callbackf(const YAML::Node& node, lua_State* L, LuaValue* value){
	game_class_data.push_back( parse_class(node) );
}

LuaValue load_class_data(lua_State* L, const FilenameList& filenames){
	LuaValue ret;

	game_class_data.clear();
	load_data_impl_template(filenames, "classes", load_class_callbackf, L,
			&ret);

	for (int i = 0; i < game_class_data.size(); i++){
		game_class_data[i].starting_stats.classtype = i;
	}

	return ret;
}
