/*
 * combat_stats.cpp:
 *  All the stats used by a combat entity.
 *  TODO: either rename this or 'stats.h' ?
 */

#include "../serialize/SerializeBuffer.h"
#include "class_data.h"
#include "combat_stats.h"

#include "items.h"
#include "projectile_data.h"
#include "stat_formulas.h"

#include "weapon_data.h"

CombatStats::CombatStats(const ClassStats& class_stats, const CoreStats& core,
		const CooldownStats& cooldowns, const Equipment& equipment,
		const std::vector<AttackStats>& attacks, float movespeed) :
		core(core), cooldowns(cooldowns), class_stats(class_stats), equipment(
				equipment), attacks(attacks), movespeed(movespeed) {

}

void CombatStats::step(GameState* gs, CombatGameInst* inst) {
	core.step();
	cooldowns.step();
	effects.step(gs, inst);
}

bool CombatStats::has_died() {
	return core.hp <= 0;
}

EffectiveStats CombatStats::effective_stats(GameState* gs,
		CombatGameInst* inst) const {
	return ::effective_stats(gs, inst, *this);
}

static void learn_class_spells(SpellsKnown& spells,
		const ClassSpellProgression& spell_progression, int level) {
	std::vector<spell_id> spells_available =
			spell_progression.spells_available_at_level(level);

	for (int i = 0; i < spells_available.size(); i++) {
		if (!spells.has_spell(spells_available[i])) {
			spells.add_spell(spells_available[i]);
		}
	}
}

void CombatStats::gain_level() {
	ClassType& ct = class_stats.class_type();

	core.hp += ct.hp_perlevel;
	core.max_hp += ct.hp_perlevel;

	core.mp += ct.mp_perlevel;
	core.max_mp += ct.mp_perlevel;

	core.defence += ct.def_perlevel;
	core.strength += ct.str_perlevel;
	core.magic += ct.mag_perlevel;
	core.willpower += ct.will_perlevel;

	core.hpregen += ct.hpregen_perlevel;
	core.mpregen += ct.mpregen_perlevel;

	class_stats.xplevel++;

	learn_class_spells(spells, ct.spell_progression, class_stats.xplevel);
}

void CombatStats::init() {
	if (class_stats.has_class()) {
		const ClassType& ct = class_stats.class_type();
		learn_class_spells(spells, ct.spell_progression, class_stats.xplevel);
	}
}

int CombatStats::gain_xp(int amnt) {
	int levels_gained = 0;
	class_stats.xp += amnt;
	while (class_stats.xp >= class_stats.xpneeded) {
		gain_level();
		levels_gained++;
		class_stats.xp -= class_stats.xpneeded;
		class_stats.xpneeded = experience_needed_formula(class_stats.xplevel);
	}
	return levels_gained;
}

bool AttackStats::is_ranged() const {
	return projectile.valid_projectile()
			|| weapon.weapon_entry().uses_projectile;
}

WeaponEntry& AttackStats::weapon_entry() const {
	return weapon.weapon_entry();
}

ProjectileEntry& AttackStats::projectile_entry() const {
	LANARTS_ASSERT(is_ranged());
	if (projectile.valid_projectile())
		return projectile.projectile_entry();

	int created = weapon.weapon_entry().created_projectile;
	LANARTS_ASSERT(created > -1);
	return game_projectile_data.at(created);
}

int AttackStats::atk_cooldown() const {
	if (projectile.valid_projectile()) {
		return projectile.projectile_entry().cooldown;
	}
	return weapon.weapon_entry().cooldown;
}

static bool is_compatible_projectile(WeaponEntry& wentry,
		ProjectileEntry& pentry) {
	return wentry.weapon_class != "unarmed"
			&& wentry.weapon_class == pentry.weapon_class;
}
int AttackStats::atk_damage(MTwist& mt, const EffectiveStats& stats) const {
	const CoreStats& core = stats.core;
	bool hasprojectile = projectile.valid_projectile();
	WeaponEntry& wentry = weapon.weapon_entry();
	int dmg = 0;

	if (!hasprojectile
			|| is_compatible_projectile(wentry, projectile.projectile_entry())) {
		dmg += wentry.damage.calculate(mt, core);
		dmg += round(wentry.percentage_magic * stats.magic.damage);
		dmg += round((1.0f - wentry.percentage_magic) * stats.physical.damage);
	}
	if (hasprojectile) {
		ProjectileEntry& pentry = projectile.projectile_entry();
		dmg += projectile.projectile_entry().damage.calculate(mt, core);
		if (!is_compatible_projectile(wentry, pentry)) {
			dmg += round(pentry.percentage_magic * stats.magic.damage);
			dmg += round(
					(1.0f - pentry.percentage_magic) * stats.physical.damage);
		}
	}
	return dmg;
}

int AttackStats::atk_power(MTwist& mt, const EffectiveStats& stats) const {
	const CoreStats& core = stats.core;
	bool hasprojectile = projectile.valid_projectile();
	WeaponEntry& wentry = weapon.weapon_entry();
	int pow = 0;

	if (!hasprojectile
			|| is_compatible_projectile(wentry, projectile.projectile_entry())) {
		WeaponEntry& wentry = weapon.weapon_entry();
		pow += wentry.power.calculate(mt, core);
		pow += round(wentry.percentage_magic * stats.magic.power);
		pow += round((1.0f - wentry.percentage_magic) * stats.physical.power);
	}
	if (hasprojectile) {
		ProjectileEntry& pentry = projectile.projectile_entry();
		pow += projectile.projectile_entry().power.calculate(mt, core);
		if (!is_compatible_projectile(wentry, pentry)) {
			pow += round(pentry.percentage_magic * stats.magic.power);
			pow += round(
					(1.0f - pentry.percentage_magic) * stats.physical.power);
		}
	}
	return pow;
}

int AttackStats::atk_percentage_magic() const {
	if (weapon.id > 0 || !projectile.valid_projectile()) {
		return weapon.weapon_entry().percentage_magic;
	}
	return projectile.projectile_entry().percentage_magic;
}

float AttackStats::atk_resist_modifier() const {
	if (weapon.id > 0 || !projectile.valid_projectile()) {
		return weapon.weapon_entry().resist_modifier;
	}
	return projectile.projectile_entry().resist_modifier;
}

void CombatStats::serialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.write(core);
	serializer.write(cooldowns);
	serializer.write(class_stats);
	equipment.serialize(serializer);
	effects.serialize(gs, serializer);
	spells.serialize(serializer);
	serializer.write_int(attacks.size());
	for (int i = 0; i < attacks.size(); i++) {
		attacks[i].projectile.serialize(serializer);
		attacks[i].weapon.serialize(serializer);
	}
	serializer.write(movespeed);
}

void CombatStats::deserialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.read(core);
	serializer.read(cooldowns);
	serializer.read(class_stats);
	equipment.deserialize(serializer);
	effects.deserialize(gs, serializer);
	spells.deserialize(serializer);
	int size;
	serializer.read_int(size);
	attacks.resize(size);
	for (int i = 0; i < size; i++) {
		attacks[i].projectile.deserialize(serializer);
		attacks[i].weapon.deserialize(serializer);
	}
	serializer.read(movespeed);
}

