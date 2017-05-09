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

static void lua_init_metatable(lua_State* L, LuaValue& value,
        effect_id id) {
    value.push();
    /* Set self as metatable*/
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -2);

    /* Set index as effect object */
    EffectEntry& entry = game_effect_data.at(id);
    entry.raw_lua_object.push();
    lua_setfield(L, -2, "__index");
    /* Pop self */
    lua_pop(L, 1);
}

static void lua_init_effect(lua_State* L, LuaValue& value, GameInst* inst, effect_id id) {
    value.init(L);
    value.newtable();
    EffectEntry& entry = game_effect_data.at(id);
    lua_init_metatable(L, value, id);
    if (entry.init_func.isnil()) {
        return;
    }
    entry.init_func.push();
    luawrap::call<void>(L, value, inst);
}

bool EffectStats::has_active_effect() const {
    for (const Effect& eff : effects) {
        if (eff.is_active()) {
            return true;
        }
    }
    return false;
}

static float draw_alpha(Effect& effect) {
    EffectEntry& eentry = game_effect_data.at(effect.id);
    if (effect.time_left() >= eentry.fade_out) {
        return 1.0f;
    }
    return effect.time_left() / eentry.fade_out;
}

Colour EffectStats::effected_colour() {
    int r = 255 * 256, g = 255 * 256, b = 255 * 256, a = 255 * 256;
    for (Effect& eff : effects) {
        if (eff.is_active()) {
            EffectEntry& eentry = game_effect_data.at(eff.id);
            Colour c = eentry.effected_colour.mute_colour(draw_alpha(eff));
            r *= c.r + 1, g *= c.g + 1, b *= c.b + 1, a *= c.a + 1;
            r /= 256, g /= 256, b /= 256, a /= 256;
        }
    }
    return Colour(r / 256, g / 256, b / 256, a / 256);
}

void EffectStats::draw_effect_sprites(GameState* gs, GameInst* inst, const Pos& p) {
    GameView& view = gs->view();
    for (Effect& eff : effects) {
        if (eff.is_active()) {
            EffectEntry& eentry = game_effect_data.at(eff.id);
            if (eentry.effected_sprite > -1) {
                Colour drawcolour(255, 255, 255, 255 * draw_alpha(eff));
                draw_sprite(view, eentry.effected_sprite, p.x, p.y,
                        drawcolour);
            }
            lua_State* L = gs->luastate();
            if (!eentry.draw_func.empty() && !eentry.draw_func.isnil()) {
                eentry.draw_func.push();
                eff.state.push();
                luawrap::push(L, inst);
                luawrap::push(L, p.x);
                luawrap::push(L, p.y);
                lua_call(L, 4, 0);
            }
        }
    }
}

bool EffectStats::can_rest() {
    for (Effect& eff : effects) {
        if (eff.is_active()) {
            EffectEntry& eentry = game_effect_data.at(eff.id);
            if (!eentry.allowed_actions.can_use_rest) {
                return false;
            }
        }
    }
    return true;
}

AllowedActions EffectStats::allowed_actions(GameState* gs) const {
    AllowedActions actions;
    for (const Effect& eff : effects) {
        if (eff.is_active()) {
            EffectEntry& ee = game_effect_data.at(eff.id);
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

    for (const Effect& eff : effects) {
        if (eff.is_active()) {
            auto& stat_func = game_effect_data.at(eff.id).stat_func;
            if (stat_func.isnil()) {
                continue;
            }
            stat_func.push();
            eff.state.push();
            luawrap::push(L, inst);

            lua_pushvalue(L, baseind);
            lua_pushvalue(L, affind);
            lua_call(L, 4, 0);
        }
    }
//    basestats = lua_get_combatstats(L, baseind);
    effective = lua_get_effectivestats(L, affind);
    lua_pop(L, 2);
    //pop base&affected
}

static void lua_effect_func_callback(lua_State* L, LuaValue& value,
        LuaValue& state, GameInst* inst, const char* name = NULL, LuaValue arg = LuaValue()) {
    if (value.empty() || value.isnil()) {
        return;
    }

    value.push();
    state.push();
    if (name != NULL) {
        lua_pushstring(L, name);
    }
    luawrap::push(L, inst);
    if (!arg.empty()) {
        arg.push();
    }
    lua_call(L, 2 + (name != NULL) + (!arg.empty()), 0);
}

void EffectStats::remove(GameState* gs, GameInst* inst, Effect* effect) {
    if (effect == NULL) {
        LANARTS_ASSERT(false);// TODO check if needed
        return;
    }
    EffectEntry& eentry = game_effect_data.at(effect->id);
    lua_State* L = gs->luastate();
    lua_effect_func_callback(L, eentry.remove_func, effect->state, inst);
}

void EffectStats::ensure_effects_active(GameState* gs, GameInst* inst, const std::vector<StatusEffect>& status_effects, const char* name) {
    for (StatusEffect status_effect : status_effects) {
         Effect& eff = get(gs, inst, status_effect.id);
         EffectEntry& entry = game_effect_data.at(eff.id);
         // Apply as a 'derived' effect, letting the effect decide how best to accumulate.
         entry.apply_derived_func.push();
         luawrap::call<void>(gs->luastate(), eff.state, inst, status_effect.args, name != NULL ? name : "");
    }
}

void EffectStats::step(GameState* gs, GameInst* inst) {
    lua_State* L = gs->luastate();
    CombatGameInst* combat_inst = dynamic_cast<CombatGameInst*>(inst);
    for (Effect& eff : effects) {
         EffectEntry& entry = game_effect_data.at(eff.id);
         entry.remove_derived_func.push();
         luawrap::call<void>(L, eff.state, inst);
    }
    // Effects inherited from being part of the object:
    ensure_effects_active(gs, inst, inst->base_status_effects(gs), "base");
    if (combat_inst != NULL) {
        auto& items = combat_inst->inventory();
        // Factor in equipment that gives effects:
        for (auto& item : items.raw_slots()) {
            if (item.is_equipped()) {
                EquipmentEntry& entry = item.equipment_entry();
                ensure_effects_active(gs, inst, entry.effect_modifiers.status_effects, entry.name.c_str());
            }
        }
    }
    // Step for every effect in the game:
    for (Effect& eff : effects) {
        if (!eff.is_active()) {
            continue;
        }
        EffectEntry& entry = game_effect_data.at(eff.id);
        lua_effect_func_callback(L, entry.step_func, eff.state, inst);
    }
}

Effect* EffectStats::get_active(effect_id id) {
    for (Effect& eff : effects) {
        if (eff.is_active() && eff.id == id) {
            return &eff;
        }
    }

    return NULL;
}

Effect* EffectStats::get_active(const char* name) {
    for (Effect& eff : effects) {
        EffectEntry& entry = game_effect_data.at(eff.id);
        if (eff.is_active() && entry.name == name) {
            return &eff;
        }
    }

    return NULL;
}

Effect& EffectStats::get(GameState* gs, GameInst* inst, effect_id id) {
    for (Effect& eff : effects) {
        if (eff.id == id) {
            return eff;
        }
    }
    effects.push_back(Effect());
    effects.back().id = id;
    lua_init_effect(gs->luastate(), effects.back().state, inst, id);

    return effects.back();
}

Effect& EffectStats::get(GameState* gs, GameInst* inst, const char* name) {
    for (Effect& eff : effects) {
        EffectEntry& entry = game_effect_data.at(eff.id);
        if (eff.is_active() && entry.name == name) {
            return eff;
        }
    }

    effects.push_back(Effect());
    effects.back().id = get_effect_by_name(name);
    lua_init_effect(gs->luastate(), effects.back().state, inst, effects.back().id);

    return effects.back();
}

bool EffectStats::has_category(const char* category) {
    for (Effect& eff : effects) {
        if (!eff.is_active()) {
            continue;
        }
        if ((game_effect_data[eff.id].category == category)) {
            return true;
        }
    }
    return false;
}

//static void lua_init_effect(lua_State* L, LuaValue& value, StatusEffect effect) {
//    value.init(L);
//    value.newtable();
//    lua_init_metatable(L, value, effect);
//}

LuaValue EffectStats::add(GameState* gs, GameInst* inst, StatusEffect status_effect) {
    lua_State* L = gs->luastate();
    Effect& eff = get(gs, inst, status_effect.id);
    EffectEntry& entry = game_effect_data.at(status_effect.id);
    entry.apply_buff_func.push();
    luawrap::call<void>(L, eff.state, inst, status_effect.args);
    LANARTS_ASSERT(!eff.state.isnil());
    return eff.state;
}

void EffectStats::serialize(GameState* gs, SerializeBuffer& serializer) {
    LuaSerializeConfig& config = gs->luaserialize_config();
    lua_State* L = gs->luastate();

    serializer.write_int(effects.size());
    for (Effect& eff : effects) {
        serializer.write_int(eff.id);
        config.encode(serializer, eff.state);
    }
}

void EffectStats::deserialize(GameState* gs, SerializeBuffer& serializer) {
    LuaSerializeConfig& config = gs->luaserialize_config();
    lua_State* L = gs->luastate();

    int size = serializer.read_int();
    for (int i = 0; i < size; i++) {
        Effect eff;
        eff.state = LuaValue(L);
        serializer.read_int(eff.id);
        config.decode(serializer, eff.state);
        lua_init_metatable(L, eff.state, eff.id);
        effects.push_back(eff);
    }
}

void EffectStats::clear() {
    effects.clear();
}

bool EffectStats::has(const char* effect_name) {
    return get_active(effect_name) != NULL;
}
