#include "item_data.h"
#include "sprite_data.h"
#include "../world/objects/GameInst.h"
#include "../world/objects/PlayerInst.h"

void use_health_potion(GameInst* inst){
	((PlayerInst*) inst)->stats().hp += 25;
}

void use_haste_scroll(GameInst* inst){
	((PlayerInst*) inst)->status_effects().add(EFFECT_HASTE, 100);
}


ItemType game_item_data[] = {
		ItemType("gold", 14, SPR_GOLD, NULL),
		ItemType("health potion", 14, SPR_POTION, &use_health_potion),
		ItemType("haste scroll", 14, SPR_SCROLL, &use_haste_scroll)
};
size_t game_item_n = sizeof(game_item_data)/sizeof(ItemType);

