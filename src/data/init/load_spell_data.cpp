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

LuaValue load_spell_data(lua_State* L, const char* filename){
	LuaValue ret;

    fstream file(filename, fstream::in | fstream::binary);
	game_spell_data.clear();

	YAML::Parser parser(file);
	YAML::Node root;
	
	parser.GetNextDocument(root);
	
	const YAML::Node& node = root;

	for(int i = 0; i < node.size(); i++){
		const YAML::Node& n = node[i];
		int seq = parse_defaulted(n, "variations", 0);
//		TileEntry entry(
//				parse_cstr(n["spell"]) ,
//				parse_cstr(n["spritefile"]),
//				parse_defaulted(n, "solid", 0)
//		);
//		game_spell_data.push_back(entry);
	}
	file.close();
	return ret;
}
