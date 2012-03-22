/*
 * sprite_data.h
 *
 *  Created on: Feb 4, 2012
 *      Author: 100397561
 */



#ifndef SPRITE_DATA_H_
#define SPRITE_DATA_H_


#include "../display/GLImage.h"
enum {
	SPR_PLAYER = 0,
	SPR_STORM_ELE = 1,
	SPR_CHICKEN = 2,
	SPR_ZOMBIE = 3,
	SPR_AMBROSIA = 4,
	SPR_AMULET = 5,
	SPR_BOOK = 6,
	SPR_GLOVES = 7,
	SPR_GOLD = 8,
	SPR_SCROLL = 9
};
struct SpriteEntry {
	const char* name;
	GLImage img;
	SpriteEntry(const char* name, const char* fname) : name(name), img(fname){}
	void init(){
		printf("Loading sprite '%s'\n", img.filename);
		init_GL_Image(&img, img.filename);
	}
};



extern SpriteEntry game_sprite_data[];
extern size_t game_sprite_n;

#define spr_player (game_sprite_data[SPR_PLAYER])
#define spr_storm_ele (game_sprite_data[SPR_STORM_ELE])
#define spr_zombie (game_sprite_data[SPR_ZOMBIE])

void init_sprite_data();



#endif /* SPRITE_DATA_H_ */
