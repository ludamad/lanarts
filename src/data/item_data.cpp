#include "item_data.h"
#include "sprite_data.h"
#include "../world/objects/GameInst.h"
#include "../world/objects/PlayerInst.h"
#include <cstring>

void use_health_potion(GameInst* inst){
	((PlayerInst*) inst)->stats().hp += 50;
}
void use_mana_potion(GameInst* inst){
	((PlayerInst*) inst)->stats().mp += 50;
}

void use_haste_scroll(GameInst* inst){
	((PlayerInst*) inst)->status_effects().add(EFFECT_HASTE, 400);
}


struct ItemAction {
	const char* name;
	item_actionf action;
	ItemAction(const char* name, item_actionf action) :
		name(name), action(action){}
};
static ItemAction game_item_actions[] = {
		ItemAction("increase_health", &use_health_potion),
		ItemAction("increase_mana", &use_mana_potion),
		ItemAction("hasten", &use_haste_scroll),
};


ItemType game_item_data[] = {
		ItemType("gold", 14, SPR_GOLD, NULL),
		ItemType("health potion", 14, SPR_POTION, &use_health_potion),
		ItemType("mana potion", 14, SPR_MANA_POTION, &use_mana_potion),
		ItemType("haste scroll", 14, SPR_SCROLL, &use_haste_scroll)
};
size_t game_item_n = sizeof(game_item_data)/sizeof(ItemType);

item_actionf get_action_by_name(const char* name){
	for (int i = 0; i < sizeof(game_item_actions)/sizeof(ItemAction); i++){
		if (strcmp(name, game_item_actions[i].name) == 0){
			return game_item_actions[i].action;
		}
	}
	return NULL;
}
