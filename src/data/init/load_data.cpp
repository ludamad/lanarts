/*
 * load_data.cpp
 *
 *  Created on: Mar 25, 2012
 *      Author: 100397561
 */

#include "../game_data.h"
#include "../../libyaml/yaml.h"

void load_tile_data(const char* filename){

    FILE *file;
    yaml_parser_t parser;
    yaml_token_t token;
    int done = 0;
    int count = 0;
    int error = 0;

    file = fopen("res/tiles.yaml", "rb");

    yaml_parser_set_input_file(&parser, file);
}
