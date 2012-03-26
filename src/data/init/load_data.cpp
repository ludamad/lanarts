/*
 * load_data.cpp
 *
 *  Created on: Mar 25, 2012
 *      Author: 100397561
 */
#include <fstream>

#include "../game_data.h"
#include <yaml-cpp/yaml.h>

void load_tile_data(const char* filename){

    std::fstream file(filename, std::fstream::in | std::fstream::binary);

	try {
		YAML::Parser parser(file);
		YAML::Node doc;
		while(parser.GetNextDocument(doc)) {
			YAML::Emitter emitter;
			emitter << doc;
			std::cout << emitter.c_str() << "\n";
		}
	} catch(const YAML::Exception& e) {
		std::cerr << e.what() << "\n";
	}
}
