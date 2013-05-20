#include <cstring>
#include <cstdio>

#include <vector>
#include <string>

#include <lcommon/unittest.h>
#include <lcommon/Timer.h>

#include "../NetConnection.h"
#include "../impl/ClientConnection.h"
#include "../impl/ServerConnection.h"

#include "../lanarts_net.h"

static const int TEST_PORT = 6112;
static const int TEST_TIMEOUT = 1000;

struct LanartsNetInitHelper {
	LanartsNetInitHelper() {
		lanarts_net_init(true);
	}
	~LanartsNetInitHelper() {
		lanarts_net_quit();
	}

};


static bool message_was_received = false;

static void message_received(receiver_t sender, void* context,
		const char* msg, size_t len) {
//	printf("Message '%s' was received\n", std::string(msg,len).c_str());
	message_was_received = true;
}

static NetConnection* create_server() {
	NetConnection* server = create_server_connection(TEST_PORT);
	CHECK(dynamic_cast<ServerConnection*>(server));
	server->initialize_connection();
	return server;
}

static NetConnection* create_client() {
	NetConnection* client = create_client_connection("localhost", TEST_PORT);
	CHECK(dynamic_cast<ClientConnection*>(client));
	client->initialize_connection();
	return client;
}

static void create_server_and_clients(NetConnection** server, NetConnection** client, int argc = 1) {
	*server = create_server();
	for (int i = 0; i < argc; i++) {
		client[i] = create_client();
	}

	(*server)->poll(message_received, NULL, TEST_TIMEOUT);

	std::vector<UDTSOCKET> server_sockets = ((ServerConnection*)*server)->get_socket_list();
	/* Check that we got our connections */
	CHECK(server_sockets.size() == argc);
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
		client->send_message("Hello World", sizeof "Hello World", NetConnection::ALL_RECEIVERS);
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
		server->send_message("Hello World", sizeof "Hello World", NetConnection::ALL_RECEIVERS);
		client->poll(message_received, NULL, TEST_TIMEOUT);
		delete server;
		delete client;
		CHECK(message_was_received);
	}

	/* Test local host latency */
	TEST(profile_server_send) {
		LanartsNetInitHelper __init_helper;

		NetConnection* server = NULL;
		NetConnection* client = NULL;
		create_server_and_clients(&server, &client);

		double total_time = 0;
		for (int i = 0; i < 1500; i++) {
			Timer timer;
			server->send_message("Hello World", sizeof "Hello World", NetConnection::ALL_RECEIVERS);
			client->poll(message_received, NULL, TEST_TIMEOUT);
			total_time += timer.get_microseconds() / 1000.0;
		}
		printf("In profile_server_send, got average=%.2f milliseconds", float(total_time/1500));
		delete server;
		delete client;
	}

	TEST(two_clients) {
		LanartsNetInitHelper __init_helper;

		NetConnection* server = NULL;
		NetConnection* clients[] = {NULL, NULL};

		create_server_and_clients(&server, clients, 2);

		server->send_message("Hello World", sizeof "Hello World", NetConnection::ALL_RECEIVERS);

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
}
