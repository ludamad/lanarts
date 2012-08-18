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

static void init_connections(NetUpdatedState& serverstate,
		NetUpdatedState& client1state, NetUpdatedState& client2state) {
	const int TEST_PORT = 6112;

	const char* servername = "server";
	const char* clientname1 = "client1";
	const char* clientname2 = "client2";

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
static void test_net_connect_affirm() {
	NetUpdatedState serverstate, client1state, client2state;
	init_connections(serverstate, client1state, client2state);
}

static void assert_game_actions_equals(GameAction& action1,
		GameAction& action2) {
	UNIT_TEST_ASSERT(action1.origin == action2.origin);
	UNIT_TEST_ASSERT(action1.act == action2.act);
	UNIT_TEST_ASSERT(action1.frame == action2.frame);
	UNIT_TEST_ASSERT(action1.level == action2.level);
}

static void send_action(GameAction& action, int frame, int player,
		NetUpdatedState& state) {
	ActionQueue queue;
	queue.push_back(action);
	state.pd.all_players()[player].action_queue.queue_actions_for_frame(queue,
			frame);
	net_send_player_actions(state.conn, frame, player, queue);
}

static void test_net_actionqueue_assert(PlayerDataEntry& pde, GameAction& act,
		int frame) {
	ActionQueue action;
	pde.action_queue.extract_actions_for_frame(action, frame);
	UNIT_TEST_ASSERT(action.size() == 1);
	assert_game_actions_equals(action.at(0), act);
}
static void test_net_action_assert(NetUpdatedState& state, GameAction& a1,
		GameAction& a2, GameAction& a3, int frame) {
	test_net_actionqueue_assert(state.pd.all_players().at(0), a1, frame);
	test_net_actionqueue_assert(state.pd.all_players().at(1), a2, frame);
	test_net_actionqueue_assert(state.pd.all_players().at(2), a3, frame);
}
static void test_net_action_sending1() {
	NetUpdatedState serverstate, client1state, client2state;
	init_connections(serverstate, client1state, client2state);

	for (int frame = 0; frame < 3; frame++) {
		GameAction serv_action(0, GameAction::action_t(0), frame, 0);
		GameAction client1_action(1, GameAction::action_t(1), frame, 1);
		GameAction client2_action(2, GameAction::action_t(2), frame, 2);

		send_action(serv_action, frame, 0, serverstate);
		send_action(client1_action, frame, 1, client1state);
		send_action(client2_action, frame, 2, client2state);

		serverstate.conn.poll_messages();
		client1state.conn.poll_messages();
		client2state.conn.poll_messages();

		test_net_action_assert(serverstate, serv_action, client1_action,
				client2_action, frame);
		test_net_action_assert(client1state, serv_action, client1_action,
				client2_action, frame);
		test_net_action_assert(client2state, serv_action, client1_action,
				client2_action, frame);
	}
}
static void test_net_action_sending2() {
	NetUpdatedState serverstate, client1state, client2state;
	init_connections(serverstate, client1state, client2state);

	for (int frame = 0; frame < 3; frame++) {
		GameAction serv_action(0, GameAction::action_t(0), frame, 0);
		GameAction client1_action(1, GameAction::action_t(1), frame, 1);
		GameAction client2_action(2, GameAction::action_t(2), frame, 2);

		send_action(serv_action, frame, 0, serverstate);
		send_action(client1_action, frame, 1, client1state);
		send_action(client2_action, frame, 2, client2state);
	}
	serverstate.conn.poll_messages();
	client1state.conn.poll_messages();
	client2state.conn.poll_messages();

	for (int frame = 0; frame < 3; frame++) {
		GameAction serv_action(0, GameAction::action_t(0), frame, 0);
		GameAction client1_action(1, GameAction::action_t(1), frame, 1);
		GameAction client2_action(2, GameAction::action_t(2), frame, 2);

		test_net_action_assert(serverstate, serv_action, client1_action,
				client2_action, frame);
		test_net_action_assert(client1state, serv_action, client1_action,
				client2_action, frame);
		test_net_action_assert(client2state, serv_action, client1_action,
				client2_action, frame);
	}
}

void adv_net_unit_tests() {
	L = lua_open();
	UNIT_TEST(test_net_connect_affirm);
	UNIT_TEST(test_net_action_sending1);
	UNIT_TEST(test_net_action_sending2);
	lua_close(L);
}
