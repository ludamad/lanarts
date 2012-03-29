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

using namespace std;

GameSettings load_settings_data(const char* filename){

	fstream file(filename, fstream::in | fstream::binary);

	GameSettings ret;
	if (file){
		try{
			YAML::Parser parser(file);
			YAML::Node root;

			parser.GetNextDocument(root);

			optional_set(root, "fullscreen", ret.fullscreen);
			optional_set(root, "regen_level_on_death", ret.regen_on_death);
			optional_set(root, "view_width", ret.view_width);
			optional_set(root, "view_height", ret.view_height);
			if (hasnode(root, "class")){
				std::string classname;
				root["class"] >> classname;
				printf("Class is %s\n", classname.c_str());
				ret.classn = get_class_by_name(classname.c_str());
			}
		} catch (const YAML::Exception& parse){
			printf("Settings Parsed Incorrectly: \n");
			printf("%s\n", parse.what());
		}
	}

	return ret;
}
