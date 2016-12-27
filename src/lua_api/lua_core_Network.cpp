/*
 * lua_net.cpp:
 *  Bindings for network-play related actions
 */

#include <vector>
#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>

#include <SDL.h>

#include "gamestate/GameState.h"
#include "gamestate/GameSettings.h"

#include "lua_api.h"

static int net_sync_message_consume(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	gs->net_connection().consume_sync_messages(gs);
	return 0;
}

static int should_send_sync_message(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
        lua_pushboolean(L, gs->game_world().should_sync_states() && gs->player_data().get_local_player_idx() == 1);
        // lua_pushboolean(L, false);
        gs->game_world().should_sync_states() = false;
	return 1;
}

static int net_sync_message_send(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	gs->net_connection().consume_sync_messages(gs);
	net_send_state_and_sync(gs->net_connection(), gs);
	return 0;
}

static int net_connections_poll(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	gs->net_connection().poll_messages();
	return 0;
}

namespace lua_api {
	void register_net_api(lua_State* L) {
		LuaValue module = lua_api::register_lua_submodule(L, "core.Network");

		module["NONE"] = (int)GameSettings::NONE;
		module["SERVER"] = (int)GameSettings::SERVER;
		module["CLIENT"] = (int)GameSettings::CLIENT;

		module["sync_message_consume"].bind_function(net_sync_message_consume);
		module["sync_message_send"].bind_function(net_sync_message_send);
		module["should_send_sync_message"].bind_function(should_send_sync_message);
		module["connections_poll"].bind_function(net_connections_poll);
	}
}
