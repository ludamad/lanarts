#include <cstring>
#include <cstdio>

#include <vector>
#include <string>

#include <lcommon/unittest.h>
#include <lcommon/Timer.h>
#include <lcommon/perf_timer.h>
#include <lcommon/strformat.h>

#include "../NetConnection.h"
#include "../impl/ClientConnection.h"
#include "../impl/ServerConnection.h"

#include "../lanarts_net.h"

static const int TEST_PORT = 6112;
static const int TEST_TIMEOUT = 100;

struct LanartsNetInitHelper {
	LanartsNetInitHelper() {
		lanarts_net_init(true);
	}
	~LanartsNetInitHelper() {
		lanarts_net_quit();
	}

};

static bool message_was_received = false;
static std::string expected_message = "Hello World";

static void message_received(receiver_t sender, void* context, const char* msg,
		size_t len) {
	CHECK(std::string(msg, len - 1) == expected_message);
	message_was_received = true;
}

NetConnection* create_server() {
	NetConnection* server = create_server_connection(TEST_PORT);
	CHECK(dynamic_cast<ServerConnection*>(server));
	auto dummy_callback = [](){bool continue_connecting = true; return continue_connecting;};
	server->initialize_connection(dummy_callback, 1);
	return server;
}

NetConnection* create_client(const char* host) {
	NetConnection* client = create_client_connection(host, TEST_PORT);
	CHECK(dynamic_cast<ClientConnection*>(client));
	auto dummy_callback = [](){bool continue_connecting = true; return continue_connecting;};
	client->initialize_connection(dummy_callback, 1);
	return client;
}

static void create_server_and_clients(NetConnection** server,
		NetConnection** client, int argc = 1) {
	*server = create_server();
	for (int i = 0; i < argc; i++) {
		client[i] = create_client("localhost");
		client[i]->poll(message_received, NULL, TEST_TIMEOUT);
	}

	/* Check that we got our connections */
	CHECK(((ServerConnection* )*server)->get_socket_list().size() == argc);
}

static void poll_n(NetConnection* connection, int times,
		packet_recv_callback message_handler, void* context = NULL) {
	int polled = 0;
	while (polled < times) {
		int got = connection->poll(message_handler, context, TEST_TIMEOUT);
		if (got == 0) {
			return; // Failure, but go on
		}
		polled += got;
	}
}

SUITE(net_tests) {

	TEST(simple_connect) {
		LanartsNetInitHelper __init_helper;

		NetConnection* server = NULL;
		NetConnection* client = NULL;
		create_server_and_clients(&server, &client);
		delete server;
		delete client;
	}

	TEST(simple_client_send) {
		LanartsNetInitHelper __init_helper;

		message_was_received = false;
		NetConnection* server = NULL;
		NetConnection* client = NULL;
		create_server_and_clients(&server, &client);
		client->send_message("Hello World", sizeof "Hello World",
				NetConnection::ALL_RECEIVERS);
		server->poll(message_received, NULL, TEST_TIMEOUT);
		delete server;
		delete client;
		CHECK(message_was_received);
	}

	TEST(simple_server_send) {
		LanartsNetInitHelper __init_helper;

		message_was_received = false;
		NetConnection* server = NULL;
		NetConnection* client = NULL;
		create_server_and_clients(&server, &client);
		server->send_message("Hello World", sizeof "Hello World",
				NetConnection::ALL_RECEIVERS);
		client->poll(message_received, NULL, TEST_TIMEOUT);
		delete server;
		delete client;
		CHECK(message_was_received);
	}

	/* Test local host latency */TEST(profile_server_send) {
		const int TEST_RUNS = 100;
		LanartsNetInitHelper __init_helper;

		NetConnection* server = NULL;
		NetConnection* client = NULL;
		create_server_and_clients(&server, &client);

		double total_time = 0;
		/*warm-up*/
		for (int i = 0; i < 10; i++) {
			printf("Warmup packet %d\n", i);
			server->send_message("Hello World", sizeof "Hello World",
					NetConnection::ALL_RECEIVERS);
			client->poll(message_received, NULL, TEST_TIMEOUT);
		}
		for (int i = 0; i < TEST_RUNS; i++) {
			perf_timer_begin("TEST");
			server->send_message("Hello World", sizeof "Hello World",
					NetConnection::ALL_RECEIVERS);
			client->poll(message_received, NULL, TEST_TIMEOUT);
			client->send_message("Hello World", sizeof "Hello World",
					NetConnection::ALL_RECEIVERS);
			server->poll(message_received, NULL, TEST_TIMEOUT);
			perf_timer_end("TEST");
		}
		perf_print_results();
//		printf("In profile_server_send, got average=%.2f milliseconds", float(timer.get_microseconds()/1000.0/TEST_RUNS));
		delete server;
		delete client;
	}

	TEST(two_clients) {
		LanartsNetInitHelper __init_helper;

		NetConnection* server = NULL;
		NetConnection* clients[] = { NULL, NULL };

		create_server_and_clients(&server, clients, 2);

		server->send_message("Hello World", sizeof "Hello World",
				NetConnection::ALL_RECEIVERS);

		message_was_received = false;
		clients[0]->poll(message_received, NULL, TEST_TIMEOUT);
		CHECK(message_was_received);

		message_was_received = false;
		clients[1]->poll(message_received, NULL, TEST_TIMEOUT);
		CHECK(message_was_received);

		delete server;
		delete clients[0];
		delete clients[1];
	}

	struct TwoClientMultiSendHelper {
		/* Different expected amount for each player */
		int peer_id;
		std::vector<int> expected;

		TwoClientMultiSendHelper(int peer_id, int num_peers) {
			this->peer_id = peer_id;
			expected.resize(num_peers, 0);
		}

		static void expect_sequential(receiver_t sender, void* _helper,
				const char* msg, size_t len) {
			TwoClientMultiSendHelper* helper =
					(TwoClientMultiSendHelper*) _helper;
			int& expect = helper->expected.at(sender);
//			printf("Peer %d got msg='%s' from %d, expected '%d' \n", helper->peer_id, std::string(msg, len-1).c_str(), sender, expect);
			int actual = atoi(std::string(msg, len - 1).c_str());
			CHECK_EQUAL(expect, actual);
			expect++;
		}

		void check(int amount) {
			for (int i = 0; i < expected.size(); i++) {
				/* We should NOT get our own messages back */
//				printf("We are peer %d testing vs peer %d\n", peer_id, i);
				CHECK_EQUAL(i == peer_id ? 0 : amount, expected[i]);
			}
		}
	};

	TEST(multisend_stress_test) {
		const int NUM_MSGS = 8;
		const int NUM_PEERS = 4;
		const int NUM_TRIALS = 100;

		LanartsNetInitHelper __init_helper;

		NetConnection* connections[NUM_PEERS];
		std::vector<TwoClientMultiSendHelper> helpers;

		create_server_and_clients(&connections[0], &connections[1],
				NUM_PEERS - 1);
		for (int i = 0; i < NUM_PEERS; i++) {
			helpers.push_back(TwoClientMultiSendHelper(i, NUM_PEERS));
		}

		for (int i = 0; i < NUM_TRIALS; i++) {
			for (int j = 0; j < NUM_PEERS; j++) {
				for (int k = 0; k < NUM_MSGS; k++) {
					std::string to_send = format("%d", i * NUM_MSGS + k);
					/* Note: For ALL_RECEIVERS, the message should NOT relay back to us */
					connections[j]->send_message(to_send.c_str(),
							to_send.size() + 1, NetConnection::ALL_RECEIVERS);
				}
			}
			for (int j = 0; j < NUM_PEERS; j++) {
				int expected_msgs = NUM_MSGS * (NUM_PEERS - 1);
				poll_n(connections[j], expected_msgs,
						TwoClientMultiSendHelper::expect_sequential,
						&helpers.at(j));
				helpers.at(j).check((i + 1) * NUM_MSGS);
			}
		}

		for (int i = 0; i < NUM_PEERS; i++) {
			delete connections[i];
		}
	}
}
