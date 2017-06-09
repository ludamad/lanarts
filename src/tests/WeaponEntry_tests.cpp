
#include <lcommon/unittest.h>
#include <lcommon/lua_lcommon.h>

#include <luawrap/luawrap.h>
#include <luawrap/calls.h>
#include <luawrap/functions.h>

#include "draw/SpriteEntry.h"
#include "stats/items/WeaponEntry.h"
#include "data/ResourceDataSet.h"

extern ResourceDataSet<SpriteEntry> game_sprite_data;

static void setup_mock_data() {
	game_sprite_data.clear();
	game_sprite_data.push_back(SpriteEntry("sprite", ldraw::Drawable()));
}

SUITE(WeaponEntry) {
	TEST(parse_lua_table) {

		std::string program = "{"
				" name = 'item', "
				" type = 'weapon_class', "
				" spr_item = 'sprite', "
				" shop_cost = {1,2}, "
				" melee_cooldown_multiplier = 3, "
				" damage_bonuses = { damage = {base = {4,5} } }, "
				" damage = {strength = 7, magic = 8, base = 9 }, "
				" on_hit_func = 10, "
				" cooldown = 11, "
				" range = 12, "
				"}";

		TestLuaState L;
		lua_register_lcommon(L);
		setup_mock_data();

		WeaponEntry item;
		item.init(0, luawrap::eval(L, program));

		CHECK("item" == item.name);
		CHECK("sprite" == res::sprite_name(item.item_sprite));
		CHECK(Range(1,2) == item.shop_cost);
		CHECK("weapon_class" == item.weapon_class);
		CHECK(3 == item.cooldown_modifiers.melee_cooldown_multiplier);
		CHECK(Range(4,5) == item.damage_modifier().damage_stats.base);

		CHECK(7 == item.attack.damage_modifiers.damage_stats.strength);
		CHECK(8 == item.attack.damage_modifiers.damage_stats.magic);
		CHECK(Range(9,9) == item.attack.damage_modifiers.damage_stats.base);
		CHECK(10 == item.action_func().get(L).to_int());
		CHECK(11 == item.attack.cooldown);
		CHECK(12 == item.attack.range);

		L.finish_check();
	}
}
