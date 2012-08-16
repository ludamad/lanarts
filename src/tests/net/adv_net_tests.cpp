/*
 * adv_net_tests.cpp:
 *  Tests sending lanarts specific messages
 */

#include <cstring>
#include <cstdio>

#include <vector>
#include <string>

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include <net/lanarts_net.h>

#include "../tests.h"

#include "../../gamestate/PlayerData.h"
#include "../../gamestate/GameState.h"
#include "../../net/GameNetConnection.h"

static lua_State* L = NULL;

struct NetUpdatedState {
	GameNetConnection conn;
	GameChat chat;
	GameStateInitData init;
	PlayerData pd;
	NetUpdatedState() :
			conn(chat, pd, init) {
	}
};

static void test_state_helper(NetUpdatedState& netstate, const char* servername,
		const char* clientname1, const char* clientname2) {
	std::vector<PlayerDataEntry>& pdes = netstate.pd.all_players();

	UNIT_TEST_ASSERT(pdes.size() == 3);
	for (int i = 0; i < pdes.size(); i++) {
		UNIT_TEST_ASSERT(pdes[i].player_inst.get_instance() == NULL);
		UNIT_TEST_ASSERT(pdes[i].net_id == i);
	}
	/* test expected names */
	UNIT_TEST_ASSERT(pdes[0].player_name == servername);
	UNIT_TEST_ASSERT(pdes[1].player_name == clientname1);
	UNIT_TEST_ASSERT(pdes[2].player_name == clientname2);
}
static void test_initstate_helper(GameStateInitData& init, int seed) {
	UNIT_TEST_ASSERT(init.seed == seed);
	UNIT_TEST_ASSERT(init.seed_set_by_network_message);
}
static void test_net_connect_affirm() {
	const int TEST_PORT = 6112;

	const char* servername = "server";
	const char* clientname1 = "client1";
	const char* clientname2 = "client2";

	NetUpdatedState serverstate, client1state, client2state;

	serverstate.conn.initialize_as_server(TEST_PORT);
	serverstate.pd.register_player("server", NULL, 0, 0);

	client1state.conn.initialize_as_client("localhost", TEST_PORT);
	client2state.conn.initialize_as_client("localhost", TEST_PORT);

	net_send_connection_affirm(client1state.conn, clientname1, 1);
	net_send_connection_affirm(client2state.conn, clientname2, 2);

	serverstate.conn.poll_messages();

	const int seed = 0x533D;
	net_send_game_init_data(serverstate.conn, serverstate.pd, seed);

	client1state.conn.poll_messages();
	client2state.conn.poll_messages();

	test_state_helper(serverstate, servername, clientname1, clientname2);

	test_state_helper(client1state, servername, clientname1, clientname2);
	test_initstate_helper(client1state.init, seed);

	test_state_helper(client2state, servername, clientname1, clientname2);
	test_initstate_helper(client2state.init, seed);
}

void adv_net_unit_tests() {
	L = lua_open();
	UNIT_TEST(test_net_connect_affirm);
	lua_close(L);
}
