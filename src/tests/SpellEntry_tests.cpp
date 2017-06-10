#include <lcommon/unittest.h>
#include <lcommon/lua_lcommon.h>

#include <luawrap/luawrap.h>
#include <luawrap/calls.h>
#include <luawrap/functions.h>

#include "data/ResourceDataSet.h"
#include "draw/SpriteEntry.h"
#include "stats/items/ProjectileEntry.h"
#include "stats/SpellEntry.h"

extern ResourceDataSet<SpriteEntry> game_sprite_data;
extern ResourceDataSet<ItemEntry*> game_item_data;

//static void setup_mock_data() {
//	game_sprite_data.clear();
//	game_sprite_data.push_back(SpriteEntry("sprite", ldraw::Drawable()));
//	for (int i = 0; i < game_item_data.size(); i++) {
//		delete game_item_data[i];
//	}
//	game_item_data.clear();
//	ProjectileEntry projectile;
//	projectile.name = "projectile";
//	game_item_data.push_back(new ProjectileEntry(projectile));
//}
//
//SUITE(SpellEntry) {
//	TEST(parse_lua_table) {
//
//		std::string program =
//				"table = {"
//						"    name = 'spell', "
//						"    spr_spell = 'sprite', "
//						"    description = 'description', "
//						"    projectile = 'projectile', "
//						"    mp_cost = 1, "
//						"    cooldown = 2, "
//						"    autotarget_func = 3, "
//						"    can_cast_with_held_key = true, "
//						"    can_cast_with_cooldown = true, "
//						"    fallback_to_melee = true "
//						"}";
//
//		TestLuaState L;
//		setup_mock_data();
//		luawrap::dostring(L, program.c_str());
//
//		SpellEntry spell;
//		spell.init(0, luawrap::globals(L)["table"]);
//		CHECK("spell" == spell.name);
//		CHECK("sprite" == res::sprite_name(spell.sprite));
//		CHECK("projectile" == spell.projectile.projectile_entry().name);
//		CHECK("description" == spell.description);
//		CHECK(1 == spell.mp_cost);
//		CHECK(2 == spell.cooldown);
//		CHECK(3 == spell.autotarget_func.get(L).to_num());
//		CHECK(true == spell.can_cast_with_held_key);
//		CHECK(true == spell.can_cast_with_cooldown);
//		CHECK(true == spell.fallback_to_melee);
//
//		L.finish_check();
//	}
//}
