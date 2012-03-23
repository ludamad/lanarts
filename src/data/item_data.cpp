#include "item_data.h"
#include "sprite_data.h"
#include "../world/objects/GameInst.h"
#include "../world/objects/PlayerInst.h"

void use_potion(GameInst* inst){
    ((PlayerInst*) inst)->stats().hp += 25;
}


ItemType game_item_data[] = {
		ItemType("gold", 14, SPR_GOLD, NULL),
		ItemType("potion", 14, SPR_POTION, &use_potion)
};
size_t game_item_n = sizeof(game_item_data)/sizeof(ItemType);

