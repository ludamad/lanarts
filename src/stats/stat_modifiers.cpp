/*
 * stat_modifiers.cpp:
 *  Various stat modifiers
 */

#include <luawrap/luawrap.h>

#include "gamestate/GameState.h"
#include "stat_modifiers.h"

void parse_magic_percentage(DamageStats& dm, const LuaField& value) {
	using namespace luawrap;

	if (!value.isnil()) {
		dm.magic_percentage = defaulted(value, "magic", 0.0f);
		dm.physical_percentage = defaulted(value, "physical", 0.0f);
	} else {
		dm.physical_percentage = 1.0f;
	}
}

DamageStats parse_damage_modifiers(const LuaField& value) {
	DamageStats dmg;
	if (value.isnil()) {
		return dmg;
	}
	dmg.damage_stats = parse_core_stat_multiplier(value["damage"]);
	dmg.power_stats = parse_core_stat_multiplier(value["power"]);
	parse_magic_percentage(dmg, value["damage_type"]);

	return dmg;
}

ArmourStats parse_defence_modifiers(const LuaField& value) {
	ArmourStats def;
	if (value.isnil()) {
		return def;
	}
	def.resistance = parse_core_stat_multiplier(value["resistance"]);
	def.magic_resistance = parse_core_stat_multiplier(
			value["magic_resistance"]);
	return def;
}

StatModifiers parse_stat_modifiers(const LuaField& value) {
	StatModifiers modifiers;
	modifiers.damage_mod = parse_damage_modifiers(value["damage_bonuses"]);
	modifiers.core_mod = parse_core_stats(value["stat_bonuses"]);
	modifiers.armour_mod = parse_defence_modifiers(value);
	return modifiers;
}

void StatusEffect::serialize(GameState* gs, SerializeBuffer& serializer) {
    serializer.write(id);
    LuaSerializeConfig& conf = gs->luaserialize_config();
    conf.encode(serializer, args);
}

void StatusEffect::deserialize(GameState* gs, SerializeBuffer& serializer) {
    serializer.read(id);
    LuaSerializeConfig& conf = gs->luaserialize_config();
    conf.decode(serializer, args);
}

void StatusEffectModifiers::serialize(GameState* gs,
        SerializeBuffer& serializer) {
    serializer.write_int(status_effects.size());
    for (auto& s : status_effects) {
        s.serialize(gs, serializer);
    }
}

void StatusEffectModifiers::deserialize(GameState* gs,
        SerializeBuffer& serializer) {
    status_effects.resize(serializer.read_int());
    for (auto& s : status_effects) {
        s.deserialize(gs, serializer);
    }
}
