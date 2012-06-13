/*
 * combat_stats.h:
 *  All the stats used by a combat entity.
 *  TODO: either rename this or 'stats.h' ?
 */

#ifndef COMBAT_STATS_H_
#define COMBAT_STATS_H_

#include <vector>

#include "effects.h"
#include "Equipment.h"
#include "stats.h"

struct lua_State;
struct GameState;

/* Represents stats related to a single attack option */
struct AttackStats {
	Weapon weapon;
	Projectile projectile;
	AttackStats(Weapon weapon = Weapon(), Projectile projectile = Projectile()) :
			weapon(weapon), projectile(projectile) {
	}
	bool is_ranged() const;
	WeaponEntry& weapon_entry() const;
	ProjectileEntry& projectile_entry() const;

	int atk_cooldown() const;

	int atk_damage(MTwist& mt, const EffectiveStats& stats) const;
	int atk_power(MTwist& mt, const EffectiveStats& stats) const;
	int atk_percentage_magic() const;
};

/* Represents all the stats used by a combat entity */
struct CombatStats {
	CoreStats core;
	CooldownStats cooldowns;
	ClassStats class_stats;
	_Equipment equipment;
	EffectStats effects;

	std::vector<AttackStats> attacks;

	float movespeed;

	CombatStats(const ClassStats& class_stats = ClassStats(),
			const CoreStats& core = CoreStats(),
			const CooldownStats& cooldowns = CooldownStats(),
			const _Equipment& equipment = _Equipment(),
			const std::vector<AttackStats>& attacks =
					std::vector<AttackStats>(), float movespeed = 0.0f) :
			core(core), cooldowns(cooldowns), class_stats(class_stats), equipment(
					equipment), attacks(attacks), movespeed(movespeed) {
	}

	void step();

	bool has_died();

	EffectiveStats effective_stats_without_atk(GameState* gs) const;

	void gain_level();
	int gain_xp(int amnt);
};

#endif /* COMBAT_STATS_H_ */
