#include "effect_data.h"
#include "../world/objects/GameInst.h"
#include "../world/objects/PlayerInst.h"

void use_haste(Stats& basestats, Stats& affected){
	affected.movespeed += basestats.movespeed;
	affected.melee.cooldown /= 2;
	affected.ranged.cooldown /= 2;
}


Effect game_effect_data[] = {
		Effect("haste", 100, &use_haste)
};
size_t game_effect_n = sizeof(game_effect_data)/sizeof(Effect);

