/*
 * Attack.cpp:
 *  Represents all the data required for an attack. Common to projectiles & melee weapons.
 */

#include <luawrap/luawrap.h>

#include "Attack.h"

Attack parse_attack(const LuaField& field) {
	using namespace luawrap;

	Attack atk;
	atk.damage_modifiers = parse_damage_modifiers(field);
	atk.cooldown = defaulted(field, "cooldown", 0);
	atk.range = defaulted(field, "range", 0);
	if (!field["on_hit_func"].isnil()) {
		atk.attack_action.action_func = LuaLazyValue(LuaValue(field["on_hit_func"]));
	}
	return atk;
}

