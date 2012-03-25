#include "effect_data.h"
#include "../world/objects/GameInst.h"
#include "../world/objects/PlayerInst.h"

void use_haste(Stats& basestats, Stats& affected){
	affected.movespeed += basestats.movespeed/2;
	affected.melee.cooldown *= 1.75;
	affected.melee.damage += 10;
	affected.ranged.cooldown *= 1.75;
	affected.ranged.damage += 10;
}


Effect game_effect_data[] = {
		Effect("haste",&use_haste)
};
size_t game_effect_n = sizeof(game_effect_data)/sizeof(Effect);

