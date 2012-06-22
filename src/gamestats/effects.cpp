extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include "../data/effect_data.h"

#include "../lua/lua_api.h"

#include "effects.h"

bool EffectStats::has_active_effect() const {
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0) {
			return true;
		}
	}
	return false;
}

void EffectStats::process(lua_State* L, const CombatStats& basestats, EffectiveStats& effective) const {
	if (!has_active_effect())
		return;
	lua_push_combatstats(L, basestats);
	lua_push_effectivestats(L, effective);

	int affind = lua_gettop(L);
	int baseind = affind-1;

	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0) {
			game_effect_data[effects[i].effect].statmod.push(L);
			lua_pushvalue(L, baseind);
			lua_pushvalue(L, affind);
			lua_call(L, 2, 0);
		}
	}
//	basestats = lua_get_combatstats(L, baseind);
	effective = lua_get_effectivestats(L, affind);
	lua_pop(L, 2);//pop base&affected
}

void EffectStats::step() {
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0)
			effects[i].t_remaining--;

	}
}

Effect* EffectStats::get(int effect) {
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0 && effects[i].effect == effect) {
			return &effects[i];
		}
	}

	return NULL;
}

void EffectStats::add(int effect, int length) {
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining == 0 || effects[i].effect == effect) {
			effects[i].effect = effect;
			effects[i].t_remaining += length;
			return;
		}
	}

}

