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

SUITE(ItemEntry) {
	TEST(parse_lua_table) {

		std::string program =
				"table = {"
						" name = 'item', "
						" spr_item = 'sprite', "
						" shop_cost = {1,2}, "
						" description = 'description', "
						" use_message = 'use_message', "
						" cant_use_message = 'cant_use_message',"
						" action_func = 1, "
						" prereq_func = 2"
						"}";

		TestLuaState L;
		lua_register_lcommon(L, luawrap::globals(L));
		setup_mock_data();
		luawrap::dostring(L, program.c_str());

		ItemEntry item;
		item.init(0, luawrap::globals(L)["table"]);
		CHECK("item" == item.name);
		CHECK("sprite" == res::sprite_name(item.item_sprite));
		CHECK(Range(1,2) == item.shop_cost);
		CHECK("description" == item.description);
		CHECK("use_message" == item.use_action.success_message );
		CHECK("cant_use_message" == item.use_action.failure_message );
		CHECK(1 == item.use_action.action_func.get(L).to_int());
		CHECK(2 == item.use_action.prereq_func.get(L).to_int());

		L.finish_check();
	}
}
