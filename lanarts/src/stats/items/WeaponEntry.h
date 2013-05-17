/*
 * WeaponEntry.h:
 *  Represents enemy & player weapon (ie, for enemies, natural attacks) data
 *  loaded from the yaml
 */

#ifndef WEAPONENTRY_H_
#define WEAPONENTRY_H_

#include "lanarts_defines.h"

#include "../Attack.h"
#include "../LuaAction.h"

#include "EquipmentEntry.h"

class WeaponEntry: public EquipmentEntry {
public:
	WeaponEntry() :
			EquipmentEntry(WEAPON), uses_projectile(false) {

	}
	virtual ~WeaponEntry() {
	}

	virtual void initialize(lua_State* L) {
		EquipmentEntry::initialize(L);
		attack.init(L);
	}
	CoreStatMultiplier& damage_stats() {
		return attack.damage_stats();
	}

	CoreStatMultiplier& power_stats() {
		return attack.power_stats();
	}

	float magic_percentage() {
		return attack.magic_percentage();
	}
	float physical_percentage() {
		return attack.physical_percentage();
	}

	float resistability() {
		return attack.resistability();
	}
	int range() {
		return attack.range;
	}

	int cooldown() {
		return attack.cooldown;
	}
	sprite_id attack_sprite() {
		return attack.attack_sprite;
	}
	LuaLazyValue& action_func() {
		return attack.attack_action.action_func;
	}

	virtual void parse_lua_table(const LuaValue& table);

	std::string weapon_class;
	bool uses_projectile;
	Attack attack;
};

weapon_id get_weapon_by_name(const char* name);
WeaponEntry& get_weapon_entry(weapon_id id);

#endif /* WEAPONENTRY_H_ */
