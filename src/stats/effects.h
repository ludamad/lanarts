#ifndef EFFECTS_H
#define EFFECTS_H

#include "../lanarts_defines.h"
#include "../lua/LuaValue.h"

const int EFFECTS_MAX = 40;

struct CombatStats;
struct EffectiveStats;
struct GameState;
struct CombatGameInst;
class SerializeBuffer;

struct Effect {
	effect_id effectid;
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
	LuaValue add(GameState* gs, CombatGameInst* inst, effect_id effect,
			int length);
	Effect* get(effect_id effect);
	void step(GameState* gs, CombatGameInst* inst);
	void draw_effect_sprites(GameState* gs, const Pos& p);
	bool can_rest();
	Colour effected_colour();

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);

	void process(GameState* gs, CombatGameInst* inst,
			EffectiveStats& effective) const;

	Effect effects[EFFECTS_MAX];
};

#endif // EFFECTS_H
