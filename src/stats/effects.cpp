#include <lua.hpp>

#include <luawrap/luawrap.h>

#include <lcommon/geometry.h>
#include <lcommon/SerializeBuffer.h>
#include <lcommon/luaserialize.h>

#include "data/lua_game_data.h"
#include "gamestate/GameState.h"

#include "lua_api/lua_api.h"

#include "objects/CombatGameInst.h"

#include "effect_data.h"

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
	if (effect.t_remaining >= eentry.fade_out) {
		return 1.0f;
	}
	return effect.t_remaining / eentry.fade_out;
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
				draw_sprite(view, eentry.effected_sprite, p.x, p.y,
						drawcolour);
			}
		}
	}
}

bool EffectStats::can_rest() {
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0) {
			EffectEntry& eentry = game_effect_data.at(effects[i].effectid);
			return false;
		}
	}
	return true;
}

AllowedActions EffectStats::allowed_actions(GameState* gs) const {
	AllowedActions actions;
	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0) {
			EffectEntry& ee = game_effect_data.at(effects[i].effectid);
			actions = actions.only_in_both(ee.allowed_actions);
		}
	}
	return actions;
}

void EffectStats::process(GameState* gs, CombatGameInst* inst,
		EffectiveStats& effective) const {
	if (!has_active_effect())
		return;
	lua_State* L = gs->luastate();
	lua_push_combatstats(L, inst->stats());
	lua_push_effectivestats(L, effective);

	int affind = lua_gettop(L);
	int baseind = affind - 1;

	for (int i = 0; i < EFFECTS_MAX; i++) {
		if (effects[i].t_remaining > 0) {
			auto& stat_func = game_effect_data.at(effects[i].effectid).stat_func;
                        if (stat_func.empty()) {
                            continue;
                        }
                        stat_func.get(L).push();
			if (!effects[i].state.empty()) {
				effects[i].state.push();
				luawrap::push(L, inst);

				lua_pushvalue(L, baseind);
				lua_pushvalue(L, affind);
				lua_call(L, 4, 0);
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
	if (value.empty() || value.isnil()) {
		return;
	}

	value.push();
	effect.state.push();
	luawrap::push(L, inst);
	lua_call(L, 2, 0);
}

void EffectStats::step(GameState* gs, CombatGameInst* inst) {
	lua_State* L = gs->luastate();
	for (int i = 0; i < EFFECTS_MAX; i++) {
		Effect& e = effects[i];
		if (e.t_remaining > 0) {
			e.t_remaining--;

			EffectEntry& eentry = game_effect_data.at(e.effectid);

			e.state["time_left"] = e.t_remaining;
			lua_effect_func_callback(L, eentry.step_func.get(L), e, inst);
			e.t_remaining = e.state["time_left"].to_int();

			if (e.t_remaining == 0) {
				lua_effect_func_callback(L, eentry.finish_func.get(L), e, inst);
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

static void lua_init_metatable(lua_State* L, LuaValue& value,
		effect_id effect) {
	value.push();
	/* Set self as metatable*/
	lua_pushvalue(L, -1);
	lua_setmetatable(L, -2);

	/* Set index as effect object */
	EffectEntry& eentry = game_effect_data.at(effect);
	lua_effects[eentry.name].push();
	lua_setfield(L, -2, "__index");
	/* Pop self */
	lua_pop(L, 1);
}

static void lua_init_effect(lua_State* L, LuaValue& value, effect_id effect) {
	value.init(L);
	value.newtable();
	lua_init_metatable(L, value, effect);
}

LuaValue EffectStats::add(GameState* gs, CombatGameInst* inst, effect_id effect,
		int length) {
	lua_State* L = gs->luastate();
        for (auto& e : effects) {
	    if (e.effectid == effect && e.t_remaining > 0) {
                // Renew an existing effect slot:
	        e.t_remaining = std::max(e.t_remaining, length);
                return e.state;
            }
        }
        for (auto& e : effects) {
	    if (e.t_remaining == 0) {
                // Init inside an empty effect slot:
		e.effectid = effect;
                lua_init_effect(L, e.state, effect);
	        e.t_remaining = length;
                e.state["time_left"] = e.t_remaining;

                EffectEntry& eentry = game_effect_data.at(e.effectid);
                lua_effect_func_callback(L, eentry.init_func.get(L), e, inst);
                return e.state;
            }
	}
	return LuaValue();
}

void EffectStats::serialize(GameState* gs, SerializeBuffer& serializer) {
	LuaSerializeConfig& config = gs->luaserialize_config();
	lua_State* L = gs->luastate();

	for (int i = 0; i < EFFECTS_MAX; i++) {
		serializer.write_int(effects[i].effectid);
		config.encode(serializer, effects[i].state);
		serializer.write_int(effects[i].t_remaining);
	}
}

void EffectStats::deserialize(GameState* gs, SerializeBuffer& serializer) {
	LuaSerializeConfig& config = gs->luaserialize_config();
	lua_State* L = gs->luastate();

	for (int i = 0; i < EFFECTS_MAX; i++) {
		serializer.read_int(effects[i].effectid);
		config.decode(serializer, effects[i].state);
		if (!effects[i].state.empty()) {
			lua_init_metatable(L, effects[i].state, effects[i].effectid);
		}
		serializer.read_int(effects[i].t_remaining);
	}
}

void EffectStats::clear() {
	for (int i = 0; i < EFFECTS_MAX; i++) {
		effects[i].t_remaining = 0;
	}
}
