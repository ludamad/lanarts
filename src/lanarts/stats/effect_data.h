#ifndef EFFECT_DATA_H_
#define EFFECT_DATA_H_

#include <cstdlib>
#include <string>
#include <vector>

#include "../lua/LuaValue.h"
#include "../lanarts_defines.h"

#include "AllowedActions.h"

#include "stats.h"

struct EffectEntry {
	std::string name;
	LuaValue stat_func, attack_stat_func, init_func, finish_func, step_func;
	Colour effected_colour;
	sprite_id effected_sprite;
	AllowedActions allowed_actions;
	bool additive_duration, fades_out;

	EffectEntry() :
			effected_sprite(-1), additive_duration(false), fades_out(false) {
	}

	void init(lua_State* L) {
		init_func.initialize(L);
		finish_func.initialize(L);
		stat_func.initialize(L);
		attack_stat_func.initialize(L);
		step_func.initialize(L);
	}
};

effect_id get_effect_by_name(const char* name);
extern std::vector<EffectEntry> game_effect_data;

#endif /* EFFECT_DATA_H_ */
