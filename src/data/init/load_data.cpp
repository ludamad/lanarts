/*
 * load_data.cpp
 *
 *  Created on: Mar 25, 2012
 *      Author: 100397561
 */
#include <fstream>

#include "../game_data.h"
#include <yaml-cpp/yaml.h>

using namespace std;

char* tocstring(string s){
	char* ret = new char[s.size()+1];
	strcpy(ret, s.c_str());
	return ret;
}

void load_tile_data(const char* filename){

    fstream file(filename, fstream::in | fstream::binary);


	YAML::Parser parser(file);
	YAML::Node root;
	
	
	string name, filen;
	int issolid = 0;
	
	parser.GetNextDocument(root);
	
	const YAML::Node& node = root["Tiles"];
	
	for(int i = 0; i < node.size(); i++){
		issolid = 0;
		const YAML::Node& entry = node[i];
		entry["tile"] >> name;
		entry["spritefile"] >> filen;
		
		if(const YAML::Node* sptr = entry.FindValue("solid")){
			entry["solid"] >> issolid;
		}
		
		printf("writing to index: %d\n", game_tile_yaml.size());
		game_tile_yaml.push_back(TileEntry(tocstring(name) , tocstring(filen), issolid));
		game_tile_yaml.back().init();
	}
}

