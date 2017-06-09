/*
 * ProjectileEntry.cpp:
 *  Represents spell/weapon/enemy's projectile data loaded from the yaml
 */

#include <cstring>

#include "ProjectileEntry.h"

#include "draw/SpriteEntry.h"
#include <luawrap/luawrap.h>

projectile_id get_projectile_by_name(const char* name) {
	if (strcmp(name, "Nothing") == 0) {
		return NO_ITEM;
	}
	item_id id = get_item_by_name(name);
	LANARTS_ASSERT(dynamic_cast<ProjectileEntry*>(game_item_data.get(id)));
	return (projectile_id)id;
}

ProjectileEntry& get_projectile_entry(projectile_id id) {
	if (id == NO_ITEM) {
		return get_projectile_entry(get_item_by_name("Nothing"));
	}
	ItemEntry* item = game_item_data.get(id);

	return dynamic_cast<ProjectileEntry&>(*item);
}

void ProjectileEntry::parse_lua_table(const LuaValue& table) {
    using namespace luawrap;
    type = EquipmentEntry::AMMO;
    EquipmentEntry::parse_lua_table(table);
    stackable = true;
    attack = parse_attack(table);
    attack.attack_sprite = item_sprite;
    if (!table["spr_attack"].isnil()) {
        attack.attack_sprite = res::sprite_id(table["spr_attack"].to_str());
    }
    drop_chance = defaulted(table, "drop_chance", 0);
    weapon_class = table["weapon_class"].to_str();

    speed = defaulted(table, "speed", 4);
    number_of_target_bounces = defaulted(table, "number_of_target_bounces", 0);
    can_wall_bounce = defaulted(table, "can_wall_bounce", false);
    can_pass_through = defaulted(table, "can_pass_through", false);
    deals_special_damage = defaulted(table, "deals_special_damage", false);
    radius = defaulted(table, "radius", 5);
}
