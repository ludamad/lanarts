#ifndef EFFECTS_H
#define EFFECTS_H

#include <luawrap/LuaValue.h>
#include <ldraw/Colour.h>

#include <vector>
#include "stats/stat_modifiers.h"
#include "lanarts_defines.h"

#include "data/lua_util.h"

#include "AllowedActions.h"
#include "effect_data.h"
#include "../../libs/luawrap-lib/include/luawrap/LuaValue.h"

const int EFFECTS_MAX = 25;

struct CombatStats;
struct EffectiveStats;
class GameState;
class GameInst;
class CombatGameInst;
class SerializeBuffer;

struct Pos;

struct Effect {
	effect_id id = -1;
    // 'state' is a plain-old Lua object that is used to control behaviour of an effect.
    // It is created the moment the effect is first used, and lives as long as its host EffectStats.
    //  'state' carries:
    //    - 'active', whether the effect is currently active
    //    - 'time_left', optionally present for effects that rely on it in the C++ side; shows remaining time until effect becomes inactive.
    //    - internal state that tracks when effects are done
    //  'state' is affected by the following methods of EffectEntry:
    //    - 'init', when first creating the effect object
    //    - 'apply_derived', apply an effect that comes from a derived status  (eg, from equipment or from other effects).
    //    - 'remove_derived', clear information accumulated from derived statuses
    //    - 'remove', clear information completely, like 'init'
    //    - 'apply_buff', apply an effect that comes from e.g. a spell or other game event. Usually interacts with 'time_left'.
    //    For other applicable methods, see EffectEntry.
	LuaValue state;
    EffectEntry& entry() const {
        return game_effect_data.get(id);
    }
    bool is_active() const {
        return !state.empty() && !state.isnil() && state["active"].as<bool>();
    }
    int time_left() const {
        if (state.empty() || state.isnil()) {
            return 0;
        }
        return state["time_left"].as<int>();
    }
    LuaField method(const char* name);
};
struct EffectStats {
    bool has_active_effect() const;
    LuaValue add(GameState* gs, GameInst* inst, StatusEffect effect);
    Effect& get(GameState* gs, GameInst* inst, effect_id effect);
    Effect& get(GameState* gs,  GameInst* inst, const char* effect_name);
    Effect* get_active(effect_id effect);
    Effect* get_active(const char* effect_name);
    bool has(const char* effect_name);
    void remove(GameState* gs, GameInst* inst, Effect* effect);
    bool has_category(const char* category);
    void ensure_effects_active(GameState* gs, GameInst* inst, const std::vector<StatusEffect>& status_effects, const char* name = NULL);
    void step(GameState* gs, GameInst* inst);
    void draw_effect_sprites(GameState* gs, GameInst* inst, const Pos& p);
    bool can_rest();
    Colour effected_colour();

    AllowedActions allowed_actions(GameState* gs) const;

    void serialize(GameState* gs, SerializeBuffer& serializer);
    void deserialize(GameState* gs, SerializeBuffer& serializer);

    // For effects interacting with the stat system:
    void process(GameState* gs, CombatGameInst* inst,
                    EffectiveStats& effective) const;

    void clear();
    template <typename F>
    void for_each(const F& f) {
        for (int i = 0; i < effects.size(); i++) {
            if (effects.at(i).is_active()) {
                f(effects.at(i));
            }
        }
    }
    std::vector<Effect> effects;
};

// Design decisions:
//  Just keep effects that were once applied. Dont bother trying to reclaim memory.
//     Rationale:
//     - Monsters will die before too much accumulation
//     - Players/applicable effects are few in number


#endif // EFFECTS_H
