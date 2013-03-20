#include <lcommon/unittest.h>
#include <lcommon/lua_lcommon.h>

#include <luawrap/luawrap.h>
#include <luawrap/calls.h>
#include <luawrap/functions.h>

#include "draw/SpriteEntry.h"
#include "stats/SpellEntry.h"

extern std::vector<SpriteEntry> game_sprite_data;

static void setup_mock_sprite() {
	game_sprite_data.clear();
	SpriteEntry entry("sprite", ldraw::Drawable());
	game_sprite_data.push_back(entry);
}

SUITE(SpellEntry_tests) {
	TEST(parse_lua_table) {

		std::string program =
				"table = {"
						"    name = 'spell', "
						"    spr_spell = 'sprite', "
						"    description = 'description', "
						"    mp_cost = 1, "
						"    cooldown = 2, "
						"    autotarget_func = 3, "
						"    can_cast_with_held_key = true, "
						"    can_cast_with_cooldown = true, "
						"    fallback_to_melee = true "
						"}";

		TestLuaState L;
		setup_mock_sprite();
		lua_safe_dostring(L, program.c_str());

		SpellEntry spell;
		spell.init(0, luawrap::globals(L)["table"]);
		CHECK("spell" == spell.name);
		CHECK("sprite" == res::sprite_name(spell.sprite));
		CHECK("description" == spell.description);
		CHECK(1 == spell.mp_cost);
		CHECK(2 == spell.cooldown);
		CHECK(3 == spell.autotarget_func.get(L).to_num());
		CHECK(true == spell.can_cast_with_held_key);
		CHECK(true == spell.can_cast_with_cooldown);
		CHECK(true == spell.fallback_to_melee);

		L.finish_check();
	}
}
