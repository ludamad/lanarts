/*
 * combat_stats.cpp:
 *  All the stats used by a combat entity.
 *  TODO: either rename this or 'stats.h' ?
 */

#include <lcommon/SerializeBuffer.h>

#include <luawrap/luawrap.h>

#include "items/ProjectileEntry.h"
#include "items/WeaponEntry.h"

#include "gamestate/GameState.h"
#include "ClassEntry.h"
#include "combat_stats.h"

#include "objects/CombatGameInst.h"

#include "items/items.h"

#include "stat_formulas.h"

CombatStats::CombatStats(const ClassStats& class_stats, const CoreStats& core,
		const CooldownStats& cooldowns, const EquipmentStats& equipment,
		const std::vector<AttackStats>& attacks, float movespeed) :
		core(core), cooldowns(cooldowns), class_stats(class_stats), equipment(
				equipment), attacks(attacks), movespeed(movespeed) {

}

void CombatStats::step(GameState* gs, CombatGameInst* inst,
		const EffectiveStats& effective_stats) {

	core.step(effective_stats.core);
	cooldowns.step(inst->is_resting);
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

void CombatStats::gain_level(CombatGameInst* inst) {
	ClassEntry& ct = class_stats.class_entry();

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
	if (!ct.on_gain_level.empty() && !ct.on_gain_level.isnil()) {
	    ct.on_gain_level.push();
	    luawrap::call<void>(ct.on_gain_level.luastate(), inst);
	}

	learn_class_spells(spells, ct.spell_progression, class_stats.xplevel);
}

void CombatStats::init() {
	if (class_stats.has_class()) {
		const ClassEntry& ct = class_stats.class_entry();
		learn_class_spells(spells, ct.spell_progression, class_stats.xplevel);
	}
}

int CombatStats::gain_xp(int amnt, CombatGameInst* inst) {
	int levels_gained = 0;
	class_stats.xp += amnt;
	while (class_stats.xp >= class_stats.xpneeded) {
		gain_level(inst);
		levels_gained++;
		class_stats.xp -= class_stats.xpneeded;
		class_stats.xpneeded = experience_needed_formula(class_stats.xplevel);
	}
    if (levels_gained > 0) {
        play("sound/Jingle_Win_Synth/Jingle_Win_Synth_02.ogg");
    }
	return levels_gained;
}

bool AttackStats::is_ranged() const {
	return !projectile.empty() || weapon.weapon_entry().uses_projectile;
}

WeaponEntry& AttackStats::weapon_entry() const {
	return weapon.weapon_entry();
}

ProjectileEntry& AttackStats::projectile_entry() const {
	LANARTS_ASSERT(is_ranged());
	LANARTS_ASSERT(!projectile.empty());
	return projectile.projectile_entry();

//	int created = weapon.weapon_entry().created_projectile;
//	LANARTS_ASSERT(created > -1);
//	return game_projectile_data.at(created);
}

int AttackStats::atk_cooldown() const {
	if (!projectile.empty()) {
		return projectile.projectile_entry().cooldown();
	}
	return weapon.weapon_entry().cooldown();
}

static bool is_compatible_projectile(WeaponEntry& wentry,
		ProjectileEntry& pentry) {
	return wentry.weapon_class != "unarmed"
			&& wentry.weapon_class == pentry.weapon_class;
}
int AttackStats::atk_damage(MTwist& mt, const EffectiveStats& stats) const {
	const CoreStats& core = stats.core;
	bool has_projectile = !projectile.empty();
	WeaponEntry& wentry = weapon.weapon_entry();
	float dmg = 0;

	if (!has_projectile
			|| is_compatible_projectile(wentry, projectile.projectile_entry())) {
		const DamageStats& dmgmod = wentry.attack.damage_modifiers;
		dmg += dmgmod.damage_stats.calculate(mt, core);
		dmg += round(dmgmod.magic_percentage * stats.magic.damage);
		dmg += round(dmgmod.physical_percentage * stats.physical.damage);
	}
	if (has_projectile) {
		ProjectileEntry& pentry = projectile.projectile_entry();
		dmg += projectile.projectile_entry().damage_stats().calculate(mt, core);
		if (!is_compatible_projectile(wentry, pentry)) {
			dmg += round(pentry.magic_percentage() * stats.magic.damage);
			dmg += round(pentry.physical_percentage() * stats.physical.damage);
		}
	}
	return dmg;
}

int AttackStats::atk_power(MTwist& mt, const EffectiveStats& stats) const {
	const CoreStats& core = stats.core;
	bool has_projectile = !projectile.empty();
	WeaponEntry& wentry = weapon.weapon_entry();
	float pow = 0;

	if (!has_projectile
			|| is_compatible_projectile(wentry, projectile.projectile_entry())) {
		WeaponEntry& wentry = weapon.weapon_entry();
                const DamageStats& dmgmod = wentry.attack.damage_modifiers;
                pow += dmgmod.power_stats.calculate(mt, core);
		pow += round(dmgmod.magic_percentage * stats.magic.power);
		pow += round(dmgmod.physical_percentage * stats.physical.power);
	}
	if (has_projectile) {
		ProjectileEntry& pentry = projectile.projectile_entry();
		float projectile_pow = pentry.power_stats().calculate(mt, core);
		pow += pentry.power_stats().calculate(mt, core);
		if (!is_compatible_projectile(wentry, pentry)) {
			pow += round(pentry.magic_percentage() * stats.magic.power);
			pow += round(pentry.physical_percentage() * stats.physical.power);
		}
	}
	return pow;
}

float AttackStats::atk_percentage_magic() const {
	if (weapon.id > 0 || projectile.empty()) {
		WeaponEntry& wentry = weapon.weapon_entry();
		DamageStats& dmgmod = wentry.attack.damage_modifiers;
		return dmgmod.magic_percentage;
	}
	return projectile.projectile_entry().magic_percentage();
}

float AttackStats::atk_percentage_physical() const {
	if (weapon.id > 0 || projectile.empty()) {
		WeaponEntry& wentry = weapon.weapon_entry();
		DamageStats& dmgmod = wentry.attack.damage_modifiers;
		return dmgmod.physical_percentage;
	}
	return projectile.projectile_entry().physical_percentage();
}

float AttackStats::atk_resist_modifier() const {
	if (weapon.id > 0 || projectile.empty()) {
		WeaponEntry& wentry = weapon.weapon_entry();
		DamageStats& dmgmod = wentry.attack.damage_modifiers;
		return dmgmod.resistability;
	}
	return projectile.projectile_entry().resistability();
}

void CombatStats::serialize(GameState* gs, SerializeBuffer& serializer) {
	serializer.write(core);
	cooldowns.serialize(gs, serializer);
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
    cooldowns.deserialize(gs, serializer);
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

AttackStats parse_attack_stats(const LuaField& value) {
	AttackStats ret;

	if (value.has("weapon")) {
		ret.weapon = Weapon(
				get_weapon_by_name(value["weapon"].to_str()));
	}
	if (value.has("projectile")) {
		ret.projectile = Projectile(
				get_projectile_by_name(value["projectile"].to_str()));
	}
	return ret;
}

CombatStats parse_combat_stats(const LuaField& value) {
	using namespace luawrap;
	CombatStats ret;

	ret.movespeed = value["movespeed"].to_num();
	if (value.has("equipment")) {
		ret.equipment = parse_equipment(value["equipment"]);
	}

	ret.core = parse_core_stats(value, true /*required*/);

	ret.class_stats.xpneeded = defaulted(value, "xpneeded",
			experience_needed_formula(1));
	ret.class_stats.xplevel = defaulted(value, "xplevel", 1);

	if (value.has("attacks")) {
	    for (int i = 0; i < value["attacks"].objlen(); i++) {
	        ret.attacks.push_back(parse_attack_stats(value["attacks"][i+1]));
	    }
	}

	return ret;
}
