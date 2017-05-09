#include <lcommon/unittest.h>
#include <lcommon/Timer.h>
#include <luawrap/testutils.h>

#include <luawrap/luawrap.h>

#include "stats/stats.h"
#include "stats/stat_modifiers.h"

static void check_stat_multiplier(const CoreStatMultiplier& sm, int offset) {
	CHECK( Range(1+offset, 2+offset) == sm.base );
	CHECK( 3+offset == sm.strength );
	CHECK( 4+offset == sm.magic );
	CHECK( 5+offset == sm.defence);
	CHECK( 6+offset == sm.willpower);
}

TEST(parse_core_stat_multiplier) {
	const char* program =
			" { base = {1,2}, strength = 3, magic = 4, defence = 5, willpower = 6 }";
	TestLuaState L;

	CoreStatMultiplier sm = parse_core_stat_multiplier(
			luawrap::eval(L, program));
	check_stat_multiplier(sm, 0);

	L.finish_check();
}

TEST(parse_cooldown_modifiers) {
	const char* program =
			" { melee_cooldown_multiplier = 1, ranged_cooldown_multiplier = 2, rest_cooldown_multiplier = 3, spell_cooldown_multiplier = 4 }";
	TestLuaState L;
	CooldownModifiers cm = parse_cooldown_modifiers(luawrap::eval(L, program));

	CHECK(1 == cm.melee_cooldown_multiplier);
	CHECK(2 == cm.ranged_cooldown_multiplier);
	CHECK(3 == cm.rest_cooldown_multiplier);
	CHECK(4 == cm.spell_cooldown_multiplier);

	L.finish_check();
}

TEST(parse_damage_modifiers) {
	const char* program =
			"{ damage = { base = {1,2}, strength = 3, magic = 4, defence = 5, willpower = 6 },"
					" power = { base = {7,8}, strength = 9, magic = 10, defence = 11, willpower = 12 },"
					" damage_type = { magic = 0.5, physical = 0.5 }}";
	TestLuaState L;
	DamageStats dmg = parse_damage_modifiers(luawrap::eval(L, program));
	check_stat_multiplier(dmg.damage_stats, 0);
	check_stat_multiplier(dmg.power_stats, 6);
	CHECK(0.5 == dmg.magic_percentage);
	CHECK(0.5 == dmg.physical_percentage);

	L.finish_check();
}

TEST(parse_defence_modifiers) {
	const char* program =
					" resistance = { base = {7,8}, strength = 9, magic = 10, defence = 11, willpower = 12 },"
					" magic_resistance = { base = {19,20}, strength = 21, magic = 22, defence = 23, willpower = 24 } }";
	TestLuaState L;
	ArmourStats def = parse_defence_modifiers(luawrap::eval(L, program));
	check_stat_multiplier(def.resistance, 6);
	check_stat_multiplier(def.magic_resistance, 18);

	L.finish_check();
}

TEST(parse_stat_modifiers) {
	const char* program =
			"{ damage_bonuses = { damage = {base = {1,2} } }, stat_bonuses = { hp = 3  } }";
	TestLuaState L;
	StatModifiers modifiers = parse_stat_modifiers(luawrap::eval(L, program));
	CHECK(Range(1,2) == modifiers.damage_mod.damage_stats.base);
	CHECK(3 == modifiers.core_mod.hp);

	L.finish_check();
}
