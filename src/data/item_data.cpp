#include "item_data.h"
#include "sprite_data.h"
#include "../world/objects/GameInst.h"
#include "../world/objects/PlayerInst.h"
#include <cstring>

void use_health_potion(GameState* gs, ItemType* item, GameInst* inst){
	((PlayerInst*) inst)->stats().hp += item->action_amount;
}
void use_mana_potion(GameState* gs, ItemType* item, GameInst* inst){
	((PlayerInst*) inst)->stats().mp += item->action_amount;
}

void use_haste_scroll(GameState* gs, ItemType* item, GameInst* inst){
	((PlayerInst*) inst)->status_effects().add(EFFECT_HASTE, item->action_duration);
}
void equip_weapon(GameState* gs, ItemType* item, GameInst* inst){
	PlayerInst* play = (PlayerInst*) inst;
	play->weapon_type() = item->weapon;
}


void increase_strength(GameState* gs, ItemType* item, GameInst* inst){
	PlayerInst* play = (PlayerInst*) inst;
	play->stats().strength += item->action_amount;
}
void increase_magic(GameState* gs, ItemType* item, GameInst* inst){
	PlayerInst* play = (PlayerInst*) inst;
	play->stats().magic += item->action_amount;
}
void increase_defence(GameState* gs, ItemType* item, GameInst* inst){
	PlayerInst* play = (PlayerInst*) inst;
	play->stats().defence += item->action_amount;
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
		ItemAction("increase_strength", &increase_strength),
		ItemAction("increase_magic", &increase_magic),
		ItemAction("increase_defence", &increase_defence),
};

item_actionf get_action_by_name(const char* name){
	for (int i = 0; i < sizeof(game_item_actions)/sizeof(ItemAction); i++){
		if (strcmp(name, game_item_actions[i].name) == 0){
			return game_item_actions[i].action;
		}
	}
	return NULL;
}
int get_item_by_name(const char* name){
	for (int i = 0; i < game_item_data.size(); i++){
		if (strcmp(name, game_item_data[i].name) == 0){
			return i;
		}
	}
	return NULL;
}
