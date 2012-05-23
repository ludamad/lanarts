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

ItemEntry parse_item_type(const YAML::Node& n){
	std::string action = parse_defaulted(n, "action", std::string());
	return ItemEntry(
			parse_cstr(n["name"]),
			parse_defaulted(n,"radius", 11),
			parse_sprite_number(n, "sprite"),
			action,
			-1,
			parse_defaulted(n,"action_amount", 50),
			parse_defaulted(n,"action_duration", 400)
		);
}
LuaValue load_item_data(lua_State* L, const char* filename){
	LuaValue ret;

	fstream file(filename, fstream::in | fstream::binary);

	if (file){
		try{
			YAML::Parser parser(file);
			YAML::Node root;


			parser.GetNextDocument(root);
			const YAML::Node& items = root["items"];
			ret.table_initialize(L);
			for (int i = 0; i < items.size(); i++){
				game_item_data.push_back(parse_item_type(items[i]));
				ret.table_set_yaml(L, game_item_data.back().name, &items[i]);
			}

			file.close();
		} catch (const YAML::Exception& parse){
			printf("Items Parsed Incorrectly: \n");
			printf("%s\n", parse.what());
		}
	}
	return ret;
}
