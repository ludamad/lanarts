/*
 * sprite_data.cpp
 */

#include "sprite_data.h"

SpriteEntry game_sprite_data[] = {
		SpriteEntry("player","res/player/fighter.png"),
		SpriteEntry("storm_ele", "res/monsters/storm_elemental.png"),
		SpriteEntry("zombie", "res/monsters/zombie.png"),
		SpriteEntry("ambrosia", "res/items/misc/ambrosia.png"),
		SpriteEntry("amulet", "res/items/misc/amulet.png"),
		SpriteEntry("book", "res/items/misc/book.png"),
		SpriteEntry("gloves", "res/items/misc/gloves.png"),
		SpriteEntry("gold", "res/items/misc/gold.png"),
		SpriteEntry("scroll", "res/items/misc/scroll.png")
};

size_t game_sprite_n = sizeof(game_sprite_data)/sizeof(SpriteEntry);

void init_sprite_data(){
	for (int i = 0; i < game_sprite_n; i++)
		game_sprite_data[i].init();
}
