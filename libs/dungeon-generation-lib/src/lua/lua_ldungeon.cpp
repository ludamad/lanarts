/*
 * lua_ldungeon.h:
 *  Bindings for the ldungeon library to Lua.
 */

#include <luawrap/luawrap.h>
#include <lcommon/lua_lcommon.h>
#include "lua_ldungeon.h"
#include "lua_ldungeon_impl.h"

namespace ldungeon_gen {

	void lua_register_map(const LuaValue& module);
	void lua_register_areatemplate(const LuaValue& module);
	void lua_register_tunnelgen(const LuaValue& module);

	void lua_register_ldungeon(const LuaValue& submodule, bool register_lcommon = true) {
		lua_State* L = submodule.luastate();
		if (register_lcommon) {
			lua_register_lcommon(L);
		}
		lua_register_map(submodule);
		lua_register_areatemplate(submodule);
		lua_register_tunnelgen(submodule);
		register_libxmi_bindings(submodule);
	}
}
