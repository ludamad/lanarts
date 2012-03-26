/*
 * load_settings_data.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: 100397561
 */





#include <fstream>

#include "load_data.h"
#include <yaml-cpp/yaml.h>
#include "yaml_util.h"

using namespace std;

template <class T>
static void optional_set(const YAML::Node& node, const char* key, T& value){
	if (hasnode(node, key)){
		node[key] >> value;
	}
}
static void optional_set(const YAML::Node& node, const char* key, bool& value){
	if (hasnode(node, key)){
		int val;
		node[key] >> val;
		value = val;
	}
}
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
		} catch (const YAML::Exception& parse){
			printf("Settings Parsed Incorrectly: \n");
			printf("%s\n", parse.what());
		}
	}

	return ret;
}
