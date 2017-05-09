/*
 * stat_modifiers.h:
 *  Various stat modifiers
 */

#ifndef STAT_MODIFIERS_H_
#define STAT_MODIFIERS_H_

#include <vector>

#include "luawrap/LuaValue.h"
#include "lanarts_defines.h"

#include "stats.h"

class LuaField;
class GameState;
class SerializeBuffer;

struct DamageStats {
	CoreStatMultiplier power_stats, damage_stats;
	float magic_percentage, physical_percentage;
	DamageStats() :
			magic_percentage(0.0f), physical_percentage(0.0f) {
	}
};

struct ArmourStats {
	CoreStatMultiplier resistance;
	CoreStatMultiplier magic_resistance;
};

//Stat modifiers that trivially stack
struct StatModifiers {
	// additive
	CoreStats core_mod;
	DamageStats damage_mod;
	ArmourStats armour_mod;
	StatModifiers() {
	}
};

DamageStats parse_damage_modifiers(const LuaField& value);
ArmourStats parse_defence_modifiers(const LuaField& value);
StatModifiers parse_stat_modifiers(const LuaField& value);

struct StatusEffect {
    effect_id id;
    LuaValue args; // What arguments do we apply to this effect?
    void serialize(GameState* gs, SerializeBuffer& serializer);
    void deserialize(GameState* gs, SerializeBuffer& serializer);
};

struct StatusEffectModifiers {
	std::vector<StatusEffect> status_effects;
	void serialize(GameState* gs, SerializeBuffer& serializer);
    void deserialize(GameState* gs, SerializeBuffer& serializer);
};

#endif /* STAT_MODIFIERS_H_ */
