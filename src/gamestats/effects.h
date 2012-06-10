#ifndef EFFECTS_H
#define EFFECTS_H

#include "../util/game_basic_structs.h"

const int EFFECTS_MAX = 40;

struct CombatStats;
struct EffectiveStats;
struct lua_State;

struct Effect {
	int effect;
	int t_remaining;
};

struct EffectStats {
	EffectStats() {
		for (int i = 0; i < EFFECTS_MAX; i++) {
			effects[i].t_remaining = 0;
		}
	}
	void add(int effect, int length);
	Effect* get(int effect);
	void step();

	void process(lua_State* L, const CombatStats& basestats, EffectiveStats& effective) const;

	Effect effects[EFFECTS_MAX];
};

#endif // EFFECTS_H
