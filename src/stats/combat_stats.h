/*
 * combat_stats.h:
 *  All the stats used by a combat entity.
 *  TODO: either rename this or 'stats.h' ?
 */

#ifndef COMBAT_STATS_H_
#define COMBAT_STATS_H_

#include <vector>

#include "Equipment.h"
#include "SpellsKnown.h"
#include "effects.h"
#include "stats.h"

struct lua_State;
class CombatGameInst;
class GameState;
class SerializeBuffer;

/* Represents stats related to a single attack option */
struct AttackStats {
	AttackStats(Item weapon = Item(), Item projectile = Item()) :
			weapon(weapon), projectile(projectile) {
	}

	bool is_ranged() const;
	WeaponEntry& weapon_entry() const;
	ProjectileEntry& projectile_entry() const;

	int atk_cooldown() const;

	int atk_damage(MTwist& mt, const EffectiveStats& stats) const;
	int atk_power(MTwist& mt, const EffectiveStats& stats) const;
	float atk_percentage_magic() const;
	float atk_percentage_physical() const;
	float atk_resist_modifier() const;

	/* members */
	Weapon weapon;
	Projectile projectile;
};

/* Represents all the stats used by a combat entity */
struct CombatStats {
	CombatStats(const ClassStats& class_stats = ClassStats(),
			const CoreStats& core = CoreStats(),
			const CooldownStats& cooldowns = CooldownStats(),
			const Equipment& equipment = Equipment(),
			const std::vector<AttackStats>& attacks =
					std::vector<AttackStats>(), float movespeed = 0.0f);

	void init();
	void step(GameState* gs, CombatGameInst* inst);

	bool has_died();

	EffectiveStats effective_stats(GameState* gs, CombatGameInst* inst) const;

	void gain_level();
	int gain_xp(int amnt);

	void serialize(GameState* gs, SerializeBuffer& serializer);
	void deserialize(GameState* gs, SerializeBuffer& serializer);

	/* members */
	CoreStats core;
	CooldownStats cooldowns;
	ClassStats class_stats;
	Equipment equipment;
	EffectStats effects;
	SpellsKnown spells;

	std::vector<AttackStats> attacks;

	float movespeed;
};

#endif /* COMBAT_STATS_H_ */
