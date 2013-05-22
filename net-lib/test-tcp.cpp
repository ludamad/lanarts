#include <cstring>
#include <cstdio>

#include <vector>
#include <string>

#include <lcommon/Timer.h>
#include <lcommon/perf_timer.h>

#include "NetConnection.h"

#include "lanarts_net.h"

struct LanartsNetInitHelper {
	LanartsNetInitHelper() {
		lanarts_net_init(true);
	}
	~LanartsNetInitHelper() {
		lanarts_net_quit();
	}

};

static const int TEST_PORT = 6112;
static const int TEST_TIMEOUT = 1000;

NetConnection* create_server() {
	NetConnection* server = create_server_connection(TEST_PORT);
	server->initialize_connection();
	return server;
}

NetConnection* create_client(const char* host) {
	NetConnection* client = create_client_connection(host, TEST_PORT);
	client->initialize_connection();
	return client;
}

static void message_received(receiver_t sender, void* context,
		const char* msg, size_t len) {
}

static void create_server_and_clients(NetConnection** server, NetConnection** client, int argc = 1) {
	*server = create_server();
	for (int i = 0; i < argc; i++) {
		client[i] = create_client("localhost");
	}

	(*server)->poll(message_received, NULL, TEST_TIMEOUT);
}


int main() {
		const int TEST_RUNS = 1000;
		LanartsNetInitHelper __init_helper;

		NetConnection* server = NULL;
		NetConnection* client = NULL;
		create_server_and_clients(&server, &client);

		double total_time = 0;
		/*warm-up*/
		for (int i = 0; i < TEST_RUNS; i++) {
			server->send_message("Hello World", sizeof "Hello World", NetConnection::ALL_RECEIVERS);
			client->poll(message_received, NULL, TEST_TIMEOUT);
		}
		for (int i = 0; i < TEST_RUNS; i++) {
			perf_timer_begin("TEST");
			server->send_message("Hello World", sizeof "Hello World", NetConnection::ALL_RECEIVERS);
			client->poll(message_received, NULL, TEST_TIMEOUT);
			client->send_message("Hello World", sizeof "Hello World", NetConnection::ALL_RECEIVERS);
			server->poll(message_received, NULL, TEST_TIMEOUT);
			perf_timer_end("TEST");
		}
		perf_print_results();
		delete server;
		delete client;

		return 0;
}
