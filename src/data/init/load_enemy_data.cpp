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

using namespace std;

Attack parse_attack(const YAML::Node& n){
	Attack ret;
	ret.canuse = true;
	ret.cooldown = parse_defaulted(n, "cooldown", 70);
	ret.range = parse_defaulted(n, "range", 25);
	ret.projectile_speed = parse_defaulted(n, "projectile_speed",0);
	n["damage"] >> ret.damage;
	ret.isprojectile= parse_defaulted(n, "projectile", 0);
	ret.attack_sprite = parse_sprite_number(n, "sprite");
	return ret;
}
EnemyType parse_enemy_type(const YAML::Node& n){
	int sprite_number;
	int radius;
	int xpaward;
	n["xpaward"] >> xpaward;
	n["radius"] >> radius;
	const YAML::Node& anodes = n["attacks"];
	vector<Attack> attacks;
	for (int i = 0; i < anodes.size(); i++){
		attacks.push_back(parse_attack(anodes[i]));
	}

	return EnemyType(
			parse_cstr(n["name"]),
			radius,
			xpaward,
			parse_sprite_number(n,"sprite"),
			parse_stats(n["stats"], attacks),
			parse_defaulted(n, "step", std::string())
	);
}

void load_enemy_data(const char* filename){

	fstream file(filename, fstream::in | fstream::binary);

	if (file){
		try{
			YAML::Parser parser(file);
			YAML::Node root;

			parser.GetNextDocument(root);
			game_enemy_data.clear();
			const YAML::Node& enemies = root["enemies"];
			for (int i = 0; i < enemies.size(); i++){
				game_enemy_data.push_back( parse_enemy_type(enemies[i]) );
			}
		} catch (const YAML::Exception& parse){
			printf("Enemies Parsed Incorrectly: \n");
			printf("%s\n", parse.what());
		}
		file.close();
	}

}
