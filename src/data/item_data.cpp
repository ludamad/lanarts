#include "item_data.h"
#include "sprite_data.h"
#include "../world/objects/GameInst.h"
#include "../world/objects/PlayerInst.h"
#include <cstring>

void use_health_potion(ItemType* item, GameInst* inst){
	((PlayerInst*) inst)->stats().hp += 50;
}
void use_mana_potion(ItemType* item, GameInst* inst){
	((PlayerInst*) inst)->stats().mp += 50;
}

void use_haste_scroll(ItemType* item, GameInst* inst){
	((PlayerInst*) inst)->status_effects().add(EFFECT_HASTE, 400);
}
void equip_weapon(ItemType* item, GameInst* inst){
	PlayerInst* play = (PlayerInst*) inst;
	play->weapon_type() = item->weapon;
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
		ItemAction("equip", &equip_weapon),
};


/*[] = {
		ItemType("gold", 14, SPR_GOLD, NULL),
		ItemType("health potion", 14, SPR_POTION, &use_health_potion),
		ItemType("mana potion", 14, SPR_MANA_POTION, &use_mana_potion),
		ItemType("haste scroll", 14, SPR_SCROLL, &use_haste_scroll)
};*/

item_actionf get_action_by_name(const char* name){
	for (int i = 0; i < sizeof(game_item_actions)/sizeof(ItemAction); i++){
		if (strcmp(name, game_item_actions[i].name) == 0){
			return game_item_actions[i].action;
		}
	}
	return NULL;
}
int get_item_by_name(const char* name){
	printf("Getting item '%s' by name\n", name);
	for (int i = 0; i < game_item_data.size(); i++){
		if (strcmp(name, game_item_data[i].name) == 0){
			printf("Comparing '%s' to '%s' returning '%d'\n", name,game_item_data[i].name,i);
			return i;
		}
	}
	return NULL;
}
