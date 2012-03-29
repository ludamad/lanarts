/*
 * load_settings_data.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: 100397561
 */





#include <fstream>

#include "../game_data.h"
#include <yaml-cpp/yaml.h>
#include "yaml_util.h"
#include "../item_data.h"

using namespace std;

ItemType parse_item_type(const YAML::Node& n){
	std::string action = parse_defaulted(n, "action", std::string());
	return ItemType(
			parse_cstr(n["name"]),
			parse_defaulted(n,"radius", 11),
			parse_sprite_number(n, "sprite"),
			get_action_by_name(action.c_str())
		);
}
void load_item_data(const char* filename){

	fstream file(filename, fstream::in | fstream::binary);

	if (file){
		try{
			YAML::Parser parser(file);
			YAML::Node root;


			parser.GetNextDocument(root);
			const YAML::Node& items = root["items"];
			for (int i = 0; i < items.size(); i++){
				game_item_data.push_back(parse_item_type(items[i]));
			}

			parser.GetNextDocument(root);

		} catch (const YAML::Exception& parse){
			printf("Items Parsed Incorrectly: \n");
			printf("%s\n", parse.what());
		}
	}

}
