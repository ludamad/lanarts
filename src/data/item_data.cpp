#include "item_data.h"
#include "sprite_data.h"


ItemType game_item_data[] = {
		ItemType("gold", 14, SPR_GOLD),
		ItemType("potion", 14, SPR_POTION)
};
size_t game_item_n = sizeof(game_item_data)/sizeof(ItemType);

