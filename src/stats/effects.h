#ifndef EFFECTS_H
#define EFFECTS_H

#include <luawrap/LuaValue.h>
#include <ldraw/Colour.h>

#include "lanarts_defines.h"

#include "AllowedActions.h"

const int EFFECTS_MAX = 25;

struct CombatStats;
struct EffectiveStats;
class GameState;
class CombatGameInst;
class SerializeBuffer;

struct Pos;

struct Effect {
	effect_id effectid = -1;
	LuaValue state;
	int t_remaining = 0;
};

struct EffectStats {
	bool has_active_effect() const;
	LuaValue add(GameState* gs, CombatGameInst* inst, effect_id effect,
			int length);
    Effect* get(effect_id effect);
    bool has_category(const char* category);
    void ensure_effects_active(GameState* gs, CombatGameInst* inst, const std::vector<effect_id>& effects, const char* name = NULL);
	void step(GameState* gs, CombatGameInst* inst);
	void draw_effect_sprites(GameState* gs, CombatGameInst* inst, const Pos& p);
	bool can_rest();
	Colour effected_colour();

	AllowedActions allowed_actions(GameState* gs) const;

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);

	void process(GameState* gs, CombatGameInst* inst,
			EffectiveStats& effective) const;

	void clear();

	Effect effects[EFFECTS_MAX];
};

#endif // EFFECTS_H
