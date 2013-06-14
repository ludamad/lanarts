#include <lcommon/unittest.h>
#include <lcommon/lua_lcommon.h>

#include <luawrap/luawrap.h>
#include <luawrap/calls.h>
#include <luawrap/functions.h>

#include "draw/SpriteEntry.h"
#include "stats/items/ProjectileEntry.h"
#include "stats/SpellEntry.h"

extern std::vector<SpriteEntry> game_sprite_data;

static void setup_mock_data() {
	game_sprite_data.clear();
	game_sprite_data.push_back(SpriteEntry("sprite", ldraw::Drawable()));
}

SUITE(EquipmentEntry) {
	TEST(parse_lua_table) {

		std::string program = "{"
				" name = 'item', "
				" type = 'helmet', "
				" spr_item = 'sprite', "
				" shop_cost = {1,2}, "
				" melee_cooldown_multiplier = 3, "
				" damage_bonuses = { damage = {base = {4,5} } }, "
				"}";

		TestLuaState L;
		lua_register_lcommon(L);
		setup_mock_data();

		EquipmentEntry item;
		item.init(0, luawrap::eval(L, program));

		CHECK("item" == item.name);
		CHECK("sprite" == res::sprite_name(item.item_sprite));
		CHECK(Range(1,2) == item.shop_cost);
		CHECK(EquipmentEntry::HEADGEAR == item.type);
		CHECK(3 == item.cooldown_modifiers.melee_cooldown_multiplier);
		CHECK(Range(4,5) == item.damage_modifier().damage_stats.base);

		L.finish_check();
	}
}
