#include <lcommon/unittest.h>
#include <lcommon/lua_lcommon.h>

#include <luawrap/luawrap.h>
#include <luawrap/calls.h>
#include <luawrap/functions.h>

#include "stats/ClassEntry.h"

SUITE(ClassEntry_tests) {
	static int merge_upvalue(lua_State* L) {
		lua_pushnil(L);

		while (lua_next(L, -2)) {

			lua_pushvalue(L, -2); // key
			lua_pushvalue(L, -2); // value
			lua_settable(L, lua_upvalueindex(1));

			lua_pop( L, 1);
			// pop value
		}
		lua_pop( L, 1);
		return 0;
	}

	static int merge_closure(lua_State* L) {
		lua_pushvalue(L, 1); // push obj
		lua_pushcclosure(L, merge_upvalue, 1);
		return 1;
	}

	static LuaValue classdefine_metatable(lua_State* L) {
		LuaValue classdefine = luameta_new(L, "ClassDefine");
		LuaValue getters = luameta_getters(classdefine);
		getters["__call"].bind_function(merge_closure);

		return classdefine;
	}
	static int class_define(lua_State* L) {
		LuaValue globals = luawrap::globals(L);
		LuaValue defined = globals["defined"].ensure_table();
		defined[lua_tostring(L, 1)] = true;
		LuaValue metatable = classdefine_metatable(L);
		lua_newtable(L);
		metatable.push();
		lua_setmetatable(L, -2);
		return 1;
	}

	TEST(class_define) {

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
		LuaValue res = globals["res"].ensure_table();

		res["class_define"].bind_function(class_define);

		lua_safe_dostring(L, program.c_str());

		L.finish_check();
	}
}
