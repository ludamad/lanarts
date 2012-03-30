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
	gainperlevel["hp"] >> hp;
	gainperlevel["mp"] >> mp;
	gainperlevel["strength"] >> str;
	gainperlevel["magic"] >> mag;
	gainperlevel["defence"] >> def;

	return ClassType(
			parse_cstr(n["name"]),
			parse_stats(n["start_stats"], attacks),
			hp, mp,
			str, def, mag
			);
}

void load_class_data(const char* filename){

	fstream file(filename, fstream::in | fstream::binary);

	if (file){
		try{
			YAML::Parser parser(file);
			YAML::Node root;

			parser.GetNextDocument(root);
			game_enemy_data.clear();
			const YAML::Node& cnode = root["classes"];
			for (int i = 0; i < cnode.size(); i++){
				game_class_data.push_back( parse_class(cnode[i]) );
				game_class_data.back().starting_stats.classtype = i;
			}
		} catch (const YAML::Exception& parse){
			printf("Class Parsed Incorrectly: \n");
			printf("%s\n", parse.what());
		}
	}
}
