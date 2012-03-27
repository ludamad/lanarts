#include <fstream>
#include "yaml_util.h"
#include "load_data.h"

using namespace std;

void load_weapon_data(const char* filename){
	
    fstream file(filename, fstream::in | fstream::binary);


	YAML::Parser parser(file);
	YAML::Node root;
	
	
	string name, filen;
	int issolid = 0;
	
	parser.GetNextDocument(root);
	game_weapon_data.clear();
	
	const YAML::Node& node = root["Weapons"];

	for(int i = 0; i < node.size(); i++){
		issolid = 0;
		const YAML::Node& n = node[i];
		
		int base_dmg, range, cooldown, proj_speed;
		float mult_dmg;
		
		n["dmg_base"] >> base_dmg;
		n["dmg_mult"] >> mult_dmg;
		n["range"] >> range;
		n["cooldown"] >> cooldown;
		
		WeaponType entry(
				parse_cstr(n["weapon"]),
				parse_defaulted(n , "projectile", 0),
				base_dmg,
				mult_dmg,
				range,
				cooldown,
				parse_sprite_number(n, "spr_weap"),
				parse_sprite_number(n,"spr_attack"),
				parse_defaulted(n , "proj_speed", 0)
		);
		
		game_weapon_data.push_back(entry);
	}
}
