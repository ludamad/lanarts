/*
 * lua_io.cpp:
 *  Bindings for network-play related actions
 */

#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>

#include <SDL.h>

#include "gamestate/GameState.h"
#include "gamestate/GameSettings.h"

#include "lua_newapi.h"

static int net_sync_message_consume(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	gs->net_connection().consume_sync_messages(gs);
	return 0;
}

static int net_sync_message_send(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	gs->net_connection().consume_sync_messages(gs);
	net_send_state_and_sync(gs->net_connection(), gs);
	return 0;
}

namespace lua_api {
	void register_net_api(lua_State* L) {
		LuaValue module = luawrap::globals(L)["net"].ensure_table();

		module["NONE"] = (int)GameSettings::NONE;
		module["SERVER"] = (int)GameSettings::SERVER;
		module["CLIENT"] = (int)GameSettings::CLIENT;

		module["sync_message_consume"].bind_function(net_sync_message_consume);
		module["sync_message_send"].bind_function(net_sync_message_send);
	}
}
