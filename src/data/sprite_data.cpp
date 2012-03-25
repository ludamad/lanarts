/*
 * sprite_data.cpp
 */

#include "sprite_data.h"

SpriteEntry game_sprite_data[] = {
		SpriteEntry("player","res/player/fighter.png"),
		SpriteEntry("storm_ele", "res/monsters/storm_elemental.png"),
		SpriteEntry("chicken", "res/monsters/chicken.png"),
		SpriteEntry("zombie", "res/monsters/zombie.png"),
		SpriteEntry("ambrosia", "res/items/misc/ambrosia.png"),
		SpriteEntry("amulet", "res/items/misc/amulet.png"),
		SpriteEntry("book", "res/items/misc/book.png"),
		SpriteEntry("gloves", "res/items/misc/gloves.png"),
		SpriteEntry("gold", "res/items/misc/gold.png"),
		SpriteEntry("scroll", "res/items/misc/scroll.png"),
		SpriteEntry("potion", "res/items/misc/redpotion.png"),
		SpriteEntry("fire", "res/effects/bolt04.png"),
		SpriteEntry("storm", "res/effects/bolt07.png"),

		SpriteEntry("grue", "res/monsters/grue.png"),
		SpriteEntry("jester", "res/monsters/jester.png"),
		SpriteEntry("super chicken", "res/monsters/superchicken.png"),
		SpriteEntry("skeleton", "res/monsters/skeleton.png"),
		SpriteEntry("ciribot", "res/monsters/ciribo.png"),
		SpriteEntry("hell forged", "res/monsters/hellforged.png"),

		SpriteEntry("mana potion", "res/items/misc/brilliant_blue.png"),
		SpriteEntry("magic blast", "res/effects/bolt05.png"),
		SpriteEntry("short sword", "res/effects/shortsword.png"),
		SpriteEntry("haste", "res/effects/haste.png")
};

size_t game_sprite_n = sizeof(game_sprite_data)/sizeof(SpriteEntry);

void init_sprite_data(){
	for (int i = 0; i < game_sprite_n; i++)
		game_sprite_data[i].init();
}
