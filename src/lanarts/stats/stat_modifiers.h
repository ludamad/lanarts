/*
 * stat_modifiers.h:
 *  Various stat modifiers
 */

#ifndef STAT_MODIFIERS_H_
#define STAT_MODIFIERS_H_

#include <vector>

#include "../lanarts_defines.h"

#include "stats.h"

struct DamageStats {
	CoreStatMultiplier power_stats, damage_stats;
	float magic_percentage, physical_percentage;
	float resistability; // How much resistance can resist this attack, lower for fast attacks
	DamageStats() :
			magic_percentage(0.0f), physical_percentage(0.0f), resistability(
					1.0f) {
	}
};

struct ArmourStats {
	CoreStatMultiplier resistance, damage_reduction;
	CoreStatMultiplier magic_resistance, magic_reduction;
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

struct StatusEffectModifiers {
	std::vector<effect_id> status_effects;
};

#endif /* STAT_MODIFIERS_H_ */
