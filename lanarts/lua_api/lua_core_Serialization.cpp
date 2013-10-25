/*
 * lua_core_Keyboard.cpp:
 *  Bindings for keyboard state queries
 */

#include <luawrap/luawrap.h>

#include "gamestate/GameState.h"

#include "lua_api.h"

static void serial_set_name_resolution_fallback(LuaStackValue callback) {
	LuaSerializeConfig& config = lua_api::gamestate(callback)->luaserialize_config();
	callback.push();
	config.index_failure_function.pop();
}

namespace lua_api {
	void register_lua_core_Serialization(lua_State* L) {
		LuaValue serial = lua_api::register_lua_submodule(L, "core.Serialization");
		LuaSerializeConfig& config = lua_api::gamestate(L)->luaserialize_config();
		serial["object_index_dictionary"] = config.obj_to_index;
		serial["index_object_dictionary"] = config.index_to_obj;
		serial["set_name_resolution_fallback"].bind_function(serial_set_name_resolution_fallback);
	}
}
