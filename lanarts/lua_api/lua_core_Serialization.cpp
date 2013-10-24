/*
 * lua_core_Keyboard.cpp:
 *  Bindings for keyboard state queries
 */

#include <luawrap/luawrap.h>

#include "gamestate/GameState.h"

#include "lua_api.h"

namespace lua_api {
	void register_lua_core_Serialization(lua_State* L) {
		LuaValue serial = lua_api::register_lua_submodule(L, "core.Serialization");
	}
}
