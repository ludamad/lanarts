/*
 * sprite_data.h
 *
 *  Created on: Feb 4, 2012
 *      Author: 100397561
 */



#ifndef SPRITE_DATA_H_
#define SPRITE_DATA_H_

#include <vector>
#include <string>

#include "../display/GLImage.h"

struct SpriteEntry {
	const char* name;
	GLImage img;
	SpriteEntry(const char* name, const char* fname) : name(name), img(fname){}
	void init(){
		init_GL_Image(&img, img.filename);
	}
};

extern std::vector<SpriteEntry> game_sprite_data;
extern size_t game_sprite_n;

inline int get_sprite_by_name(const std::string& s){
	for (int i = 0; i < game_sprite_data.size(); i++){
		if (s == game_sprite_data[i].name){
			return i;
		}
	}
	return -1;
}

void init_sprite_data();



#endif /* SPRITE_DATA_H_ */
