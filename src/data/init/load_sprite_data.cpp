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

void load_tile_data(const char* filename) {

	fstream file(filename, fstream::in | fstream::binary);
	game_tile_data.clear();


	try {
		YAML::Parser parser(file);
		YAML::Node root;
		parser.GetNextDocument(root);

		const YAML::Node& node = root["tiles"];

		for (int i = 0; i < node.size(); i++) {
			const YAML::Node& n = node[i];
			int seq = parse_defaulted(n, "variations", 0);
			bool is_seq = seq > 0;
			if (is_seq) {
				for (int i = 0; i < seq; i++) {
					char number[12];
					std::string tilefile;
					n["file"] >> tilefile;
					snprintf(number, 12, "%d", i);
					tilefile += number;
					tilefile += parse_defaulted(n, "extension", std::string());

//				printf("Parsing tile name '%s'\n", tilefile.c_str());

					std::string tilename;
					n["name"] >> tilename;
					tilename += number;

					TileEntry entry(tocstring(tilename),
							tocstring(tilefile)
					);
					game_tile_data.push_back(entry);
				}
			} else {
				TileEntry entry(parse_cstr(n["name"]),
						parse_cstr(n["file"])
				);
				game_tile_data.push_back(entry);
			}
		}
		for (int i = 0; i < game_tile_data.size(); i++) {
			game_tile_data[i].init();
		}
	} catch (const YAML::Exception& parse) {
		printf("Sprites Parsed Incorrectly: \n");
		printf("%s\n", parse.what());
	}
	file.close();
}

LuaValue load_sprite_data(lua_State* L, const char* filename) {
	LuaValue ret;

	fstream file(filename, fstream::in | fstream::binary);

	try {
		YAML::Parser parser(file);
		YAML::Node root;

		string name, filen;
		int issolid = 0;

		parser.GetNextDocument(root);
		game_sprite_data.clear();

		const YAML::Node& node = root["sprites"];

		for (int i = 0; i < node.size(); i++) {
			issolid = 0;
			const YAML::Node& n = node[i];

			SpriteEntry entry(parse_cstr(n["name"]), parse_cstr(n["file"]));

			game_sprite_data.push_back(entry);
			ret.table_set_yaml(L, game_sprite_data.back().name, &n);
		}
		for (int i = 0; i < game_sprite_data.size(); i++) {
			game_sprite_data[i].init();
		}
	} catch (const YAML::Exception& parse) {
		printf("Sprites Parsed Incorrectly: \n");
		printf("%s\n", parse.what());
	}
	file.close();

	return ret;
}

