#include <fstream>
#include "yaml_util.h"
#include "../game_data.h"
#include "../item_data.h"

using namespace std;

void load_weapon_data(const char* filename) {

	fstream file(filename, fstream::in | fstream::binary);

	YAML::Parser parser(file);
	YAML::Node root;

	string name, filen;
	int issolid = 0;
	try {
		parser.GetNextDocument(root);
		game_weapon_data.clear();

		const YAML::Node& node = root["weapons"];

		for (int i = 0; i < node.size(); i++) {
			issolid = 0;
			const YAML::Node& n = node[i];

			GenRange damage = parse_range(n["damage"]);
			WeaponType entry(parse_cstr(n["weapon"]),
					parse_defaulted(n, "projectile", 0),
					damage.min,
					damage.max,
					parse_modifiers(n["modifiers"]),
					parse_defaulted(n, "range", 25),
					parse_defaulted(n, "cooldown", 30),
					parse_sprite_number(n, "spr_weap"),
					parse_sprite_number(n, "spr_attack"),
					parse_defaulted(n, "proj_speed", 0));

			game_weapon_data.push_back(entry);
		}
		file.close();

	} catch (const YAML::Exception& parse) {
		printf("Weapons Parsed Incorrectly: \n");
		printf("%s\n", parse.what());
	}
}

void load_weapon_item_entries(){
	//Create items from weapons
	for (int i = 0; i < game_weapon_data.size(); i++){
		WeaponType* wtype = &game_weapon_data[i];
		//printf("index = %d, sprite = '%s'\n", game_item_data.size(), wtype->name);
		game_item_data.push_back(ItemType(wtype->name, 11, wtype->weapon_sprite, get_action_by_name("equip"), i));
	}
}
