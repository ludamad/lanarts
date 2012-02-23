/*
 * sprite_data.cpp
 */

#include "sprite_data.h"

SpriteEntry game_sprite_data[] = {
		SpriteEntry("player","res/player/fighter.png"),
		SpriteEntry("storm_ele", "res/monsters/storm_elemental.png"),
		SpriteEntry("zombie", "res/monsters/zombie.png"),
		SpriteEntry("ambrosia", "res/monsters/ambrosia.png"),
		SpriteEntry("amulet", "res/monsters/amulet.png"),
		SpriteEntry("book", "res/monsters/book.png"),
		SpriteEntry("gloves", "res/monsters/gloves.png"),
		SpriteEntry("gold", "res/monsters/gold.png"),
		SpriteEntry("scroll", "res/monsters/scroll.png")
};

size_t game_sprite_n = sizeof(game_sprite_data)/sizeof(SpriteEntry);

void init_sprite_data(){
	for (int i = 0; i < game_sprite_n; i++)
		game_sprite_data[i].init();
}
