#include <lcommon/unittest.h>
#include <lcommon/lua_lcommon.h>

#include <luawrap/luawrap.h>
#include <luawrap/calls.h>
#include <luawrap/functions.h>

#include "stats/ClassEntry.h"

SUITE(ClassEntry_tests) {
	static int merge_tables(lua_State* L) {
		lua_pushnil(L);

		while (lua_next(L, 2)) {
			lua_pushvalue(L, -2); // key
			lua_pushvalue(L, -2); // value
			lua_settable(L, 1);

			lua_pop( L, 1);
			// pop value
		}
		lua_pop( L, 1);
		return 0;
	}

	static void set_classdefine_metatable(lua_State* L, int idx) {
		lua_pushvalue(L, idx);

		lua_newtable(L);
		lua_pushcfunction(L, merge_tables);
		lua_setfield(L, -2, "__call");

		/* Set metatable of table at 'idx' */
		lua_setmetatable(L, -2);

		lua_pop(L, 1);
	}

	static int class_define(lua_State* L) {
		LuaValue globals = luawrap::globals(L);
		LuaValue defined = globals["definitions"].ensure_table();
		LuaValue newclass = defined[lua_tostring(L, 1)].ensure_table();

		newclass.push();
		set_classdefine_metatable(L, -1);

		return 1;
	}

	TEST(proof_of_concept) {

		std::string program = "res.class_define \"Mage\" {\n"
				"    sprites = { \"wizard\", \"wizard2\" },\n"
				"   available_spells = {\n"
				"    },\n"
				"    start_stats = {\n"
				"        movespeed = 4,\n"
				"        hp = 110,\n"
				"        mp = 40,\n"
				"        hpregen = 3.3, --per second\n"
				"        mpregen = 2.64,\n"
				"        strength = 6,\n"
				"        defence = 6,\n"
				"        willpower = 3,\n"
				"        magic = 2,\n"
				"        equipment = {\n"
				"        }\n"
				"    },\n"
				"    on_level_gain = function(obj, stats)\n"
				"        stats.hp = stats.hp + 15\n"
				"        stats.mp = stats.mp + 20\n"
				"        stats.hpregen = stats.hpregen + 0.010\n"
				"        stats.mpregen = stats.mpregen + 0.007\n"
				"        stats.magic = stats.magic + 2\n"
				"        stats.strength = stats.strength + 1\n"
				"        stats.defence = stats.defence + 1\n"
				"        stats.willpower = stats.willpower + 2\n"
				"    end\n"
				"}";

		TestLuaState L;

		LuaValue globals = luawrap::globals(L);
		LuaValue defines = globals["definitions"].ensure_table();
		LuaValue res = globals["res"].ensure_table();

		res["class_define"].bind_function(class_define);

		lua_safe_dostring(L, program.c_str());
		CHECK(!defines["Mage"].isnil());

		LuaValue mage = defines["Mage"];

		CHECK(!mage["sprites"].isnil());
		CHECK(!mage["available_spells"].isnil());
		CHECK(!mage["start_stats"].isnil());
		CHECK(!mage["on_level_gain"].isnil());

		L.finish_check();
	}
}
