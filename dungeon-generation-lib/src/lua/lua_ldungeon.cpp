/*
 * lua_ldungeon.h:
 *  Bindings for the ldungeon library to Lua.
 */

#include <luawrap/luawrap.h>
#include <lcommon/lua_lcommon.h>
#include "lua_ldungeon.h"

namespace ldungeon_gen {

	void lua_register_map(const LuaValue& module, MTwist* mtwist);
	void lua_register_areatemplate(const LuaValue& module);
	void lua_register_tunnelgen(const LuaValue& module);
	void lua_register_mapfills(const LuaValue& module);

	MTwist& ldungeon_get_rng(lua_State* L) {
		//Get RNG setup for map generation
		luawrap::registry(L)["MapGenRNG"].push();
		MTwist* mtwist = (MTwist*) lua_touserdata(L, -1);
		lua_pop(L, 1); /* pop RNG object */
		return *mtwist;
	}

	void lua_register_ldungeon(const LuaValue& module, MTwist* mtwist, bool register_lcommon = true) {
		lua_State* L = module.luastate();
		if (register_lcommon) {
			lua_register_lcommon(L);
		}
		lua_register_map(module, mtwist);
		lua_register_areatemplate(module);
		lua_register_tunnelgen(module);
		lua_register_mapfills(module);
	}
}
