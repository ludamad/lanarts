extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include "../data/effect_data.h"

#include "../lua/lua_api.h"

#include "../world/GameState.h"

#include "../world/objects/CombatGameInst.h"

#include "effects.h"

bool EffectStats::has_active_effect() const {
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0) {
			return true;
		}
	}
	return false;
}

void EffectStats::process(GameState* gs, CombatGameInst* inst,
		EffectiveStats& effective) const {
	if (!has_active_effect())
		return;
	lua_State* L = gs->get_luastate();
	lua_push_combatstats(L, inst->stats());
	lua_push_effectivestats(L, effective);

	int affind = lua_gettop(L);
	int baseind = affind - 1;

	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0) {
			game_effect_data.at(effects[i].effect).stat_func.push(L);
			effects[i].state.push(L);
			lua_push_gameinst(L, inst);

			lua_pushvalue(L, baseind);
			lua_pushvalue(L, affind);
			lua_call(L, 4, 0);
		}
	}
//	basestats = lua_get_combatstats(L, baseind);
	effective = lua_get_effectivestats(L, affind);
	lua_pop(L, 2);
	//pop base&affected
}

static void lua_effectstep_callback(lua_State* L, Effect& effect,
		CombatGameInst* inst) {
	game_effect_data.at(effect.effect).step_func.push(L);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		return;
	}
	effect.state.push(L);
	lua_push_gameinst(L, inst);
	lua_call(L, 2, 0);
}

void EffectStats::step(GameState* gs, CombatGameInst* inst) {
	lua_State* L = gs->get_luastate();
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0) {
			effects[i].t_remaining--;
			lua_effectstep_callback(L, effects[i], inst);
			if (effects[i].t_remaining == 0) {
				effects[i].state = LuaValue();
			}
		}
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

