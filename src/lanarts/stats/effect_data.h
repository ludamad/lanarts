#ifndef EFFECT_DATA_H_
#define EFFECT_DATA_H_

#include <cstdlib>
#include <string>
#include <vector>

#include <common/lua/LuaValue.h>

#include "../lua/luaexpr.h"
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
		luavalue_call_and_store(L, init_func);
		luavalue_call_and_store(L, finish_func);
		luavalue_call_and_store(L, stat_func);
		luavalue_call_and_store(L, attack_stat_func);
		luavalue_call_and_store(L, step_func);
	}
};

effect_id get_effect_by_name(const char* name);
extern std::vector<EffectEntry> game_effect_data;

#endif /* EFFECT_DATA_H_ */
