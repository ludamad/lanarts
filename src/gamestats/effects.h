#ifndef EFFECTS_H
#define EFFECTS_H

#include "../util/game_basic_structs.h"
#include "../util/LuaValue.h"

const int EFFECTS_MAX = 40;

struct CombatStats;
struct EffectiveStats;
struct GameState;
struct CombatGameInst;

struct Effect {
	effect_id effect;
	LuaValue state;
	int t_remaining;
};

struct EffectStats {
	EffectStats() {
		for (int i = 0; i < EFFECTS_MAX; i++) {
			effects[i].t_remaining = 0;
		}
	}
	bool has_active_effect() const;
	void add(effect_id effect, int length);
	Effect* get(effect_id effect);
	void step(GameState* gs, CombatGameInst* inst);

	void process(GameState* gs, CombatGameInst* inst,
			EffectiveStats& effective) const;

	Effect effects[EFFECTS_MAX];
};

#endif // EFFECTS_H
