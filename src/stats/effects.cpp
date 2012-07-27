extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include "effect_data.h"
#include "../data/lua_game_data.h"

#include "../lua/lua_api.h"

#include "../gamestate/GameState.h"

#include "../objects/CombatGameInst.h"

#include "effects.h"

bool EffectStats::has_active_effect() const {
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0) {
			return true;
		}
	}
	return false;
}

static float draw_alpha(Effect& effect) {
	EffectEntry& eentry = game_effect_data.at(effect.effectid);
	if (!eentry.fades_out || effect.t_remaining > 100) {
		return 1.0f;
	}
	return effect.t_remaining / 100.0f;
}

Colour EffectStats::effected_colour() {
	int r = 255 * 256, g = 255 * 256, b = 255 * 256, a = 255 * 256;
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0) {
			EffectEntry& eentry = game_effect_data.at(effects[i].effectid);
			Colour c = eentry.effected_colour.mute_colour(
					draw_alpha(effects[i]));
			r *= c.r + 1, g *= c.g + 1, b *= c.b + 1, a *= c.a + 1;
			r /= 256, g /= 256, b /= 256, a /= 256;
		}
	}
	return Colour(r / 256, g / 256, b / 256, a / 256);
}

void EffectStats::draw_effect_sprites(GameState* gs, const Pos& p) {
	GameView& view = gs->view();
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0) {
			EffectEntry& eentry = game_effect_data.at(effects[i].effectid);
			if (eentry.effected_sprite > -1) {
				Colour drawcolour(255, 255, 255, 255 * draw_alpha(effects[i]));
				gl_draw_sprite(view, eentry.effected_sprite, p.x, p.y,
						drawcolour);
			}
		}
	}
}

bool EffectStats::can_rest() {
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0) {
			EffectEntry& eentry = game_effect_data.at(effects[i].effectid);
			if (!eentry.can_rest) {
				return false;
			}
		}
	}
	return true;
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
			game_effect_data.at(effects[i].effectid).stat_func.push(L);
			if (!lua_isnil(L, -1)) {
				effects[i].state.push(L);
				lua_push_gameinst(L, inst);

				lua_pushvalue(L, baseind);
				lua_pushvalue(L, affind);
				lua_call(L, 4, 0);
			} else {
				lua_pop(L, 1);
			}
		}
	}
//	basestats = lua_get_combatstats(L, baseind);
	effective = lua_get_effectivestats(L, affind);
	lua_pop(L, 2);
	//pop base&affected
}

static void lua_effect_func_callback(lua_State* L, LuaValue& value,
		Effect& effect, CombatGameInst* inst) {
	value.push(L);
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
			EffectEntry& eentry = game_effect_data.at(effects[i].effectid);
			effects[i].t_remaining--;
			lua_effect_func_callback(L, eentry.step_func, effects[i], inst);
			if (effects[i].t_remaining == 0) {
				lua_effect_func_callback(L, eentry.finish_func, effects[i],
						inst);
				effects[i].state = LuaValue();
			}
		}
	}
}

Effect* EffectStats::get(int effect) {
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0 && effects[i].effectid == effect) {
			return &effects[i];
		}
	}

	return NULL;
}

static void lua_init_effect(lua_State* L, LuaValue& value, effect_id effect) {
	EffectEntry& eentry = game_effect_data.at(effect);
	value.table_initialize(L);
	value.push(L);
	/* Set self as metatable*/
	lua_pushvalue(L, -1);
	lua_setmetatable(L, -2);
	/* Set index as effect object */
	lua_effects.table_push_value(L, eentry.name.c_str());
	lua_setfield(L, -2, "__index");
	/* Call init function */
	if (!eentry.init_func.empty()) {
		eentry.init_func.push(L);
		value.push(L);
		lua_call(L, 1, 0);
	}
	/* Pop self */
	lua_pop(L, 1);
}

LuaValue EffectStats::add(GameState* gs, effect_id effect, int length) {
	lua_State* L = gs->get_luastate();
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining == 0 || effects[i].effectid == effect) {
			effects[i].effectid = effect;
			effects[i].t_remaining = std::max(effects[i].t_remaining, length);
			if (effects[i].state.empty()) {
				lua_init_effect(L, effects[i].state, effect);
			}
			return effects[i].state;
		}
	}
	return LuaValue();
}

