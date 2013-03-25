#include <lcommon/unittest.h>
#include <lcommon/lua_lcommon.h>

#include <luawrap/luawrap.h>
#include <luawrap/calls.h>
#include <luawrap/functions.h>

#include "stats/ClassEntry.h"

SUITE(ClassEntry) {
	TEST(parse_lua_table) {

		std::string program = "table = {\n"
				"    name = \"Mage\","
				"    sprites = { },\n"
				"   available_spells = {\n"
				"    },\n"
				"    start_stats = {\n"
				"        movespeed = 1,\n"
				"        hp = 2,\n"
				"        mp = 3,\n"
				"        hpregen = 0.1,\n"
				"        mpregen = 0.2,\n"
				"        strength = 4,\n"
				"        defence = 5,\n"
				"        willpower = 6,\n"
				"        magic = 7,\n"
				"        equipment = {\n"
				"        }\n"
				"    },\n"
				"    gain_per_level = {}\n"
				"}";

		TestLuaState L;
		lua_safe_dostring(L, program.c_str());

		ClassEntry mage;
		mage.init(0, luawrap::globals(L)["table"]);
		CHECK("Mage" ==  mage.name);
		CHECK(1 ==  mage.starting_stats.movespeed);
		CHECK(2 ==  mage.starting_stats.core.hp);
		CHECK(3 ==  mage.starting_stats.core.mp);
		CHECK(4 ==  mage.starting_stats.core.strength);
		CHECK(5 ==  mage.starting_stats.core.defence);
		CHECK(6 ==  mage.starting_stats.core.willpower);
		CHECK(7 ==  mage.starting_stats.core.magic);

		L.finish_check();
	}
}
