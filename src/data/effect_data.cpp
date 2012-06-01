#include "effect_data.h"
#include "../world/objects/GameInst.h"
#include "../world/objects/PlayerInst.h"

void use_haste(Stats& basestats, Stats& affected){
	affected.movespeed += basestats.movespeed/2;
	affected.meleeatk.cooldown /= 1.50;
	affected.strength += 10;
	affected.magic += 10;
	affected.defence += 10;
	affected.magicatk.cooldown /= 1.50;
}


std::vector<EffectType> init_effects(){
	std::vector<EffectType> ret;
	ret.push_back(EffectType("haste",&use_haste));
	return ret;
}

std::vector<EffectType> game_effect_data = init_effects();

