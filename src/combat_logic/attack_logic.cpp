/*
 * attack_logic.cpp:
 *  Represents melee and spell decisions for an entity
 */

#include <algorithm>

#include "../gamestats/combat_stats.h"

#include "../data/weapon_data.h"

#include "../util/math_util.h"

#include "../world/GameState.h"
#include "../world/objects/CombatGameInst.h"

#include "attack_logic.h"

const int TOO_LARGE_RANGE = 99999;
bool attack_ai_choice(GameState* gs, CombatGameInst* inst,
		CombatGameInst* target, AttackStats& attack) {
	CombatStats& stats = inst->stats();
	std::vector<AttackStats>& attacks = stats.attacks;

	int attack_id = -1;
	int smallest_range = TOO_LARGE_RANGE;
	float dist = distance_between(Pos(inst->x, inst->y),
			Pos(target->x, target->y));
	int radii = inst->target_radius + target->target_radius;

	for (int i = 0; i < attacks.size(); i++) {
		WeaponEntry& wentry = attacks[i].weapon.weapon_entry();
		int range = wentry.range;
		if (attacks[i].projectile.valid_projectile()) {
			ProjectileEntry& pentry = attacks[i].projectile.projectile_entry();
			range = std::max(range, pentry.range);
		}
		if (radii + range >= dist && range < smallest_range) {
			attack_id = i;
			smallest_range = range;
		}
	}

	if (attack_id > -1) {
		attack = attacks[attack_id];
		return true;
	}
	return false;
}

