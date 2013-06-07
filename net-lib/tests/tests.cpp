#include <cstdio>
#include <cassert>
#include <lcommon/unittest.h>
#include "../lanarts_net.h"
#include <lcommon/Timer.h>

NetConnection* create_server();
NetConnection* create_client(const char* host);

static int packet_gets = 0;
static int packet_expects = 0;

static void message_received(receiver_t sender, void* context,
		const char* msg, size_t len) {
	packet_gets++;
}

static void get_packet(NetConnection* conn) {
	packet_expects++;
	while (packet_gets < packet_expects) {
		if (!conn->poll(message_received, NULL, -1)) {
			return;
		}
	}
}

static void send_packet(NetConnection* conn) {
	conn->send_message("Hello World", sizeof "Hello World", NetConnection::ALL_RECEIVERS);
}

static int run_client_server_test(int argc, char** argv) {
	lanarts_net_init(true);
	bool isclient = (argc >= 2);
	NetConnection* conn;
	if (isclient) {
		conn = create_client(argv[1]);
	} else {
		conn = create_server();
		conn->poll(message_received, NULL, -1);
	}
	if (isclient) {
		send_packet(conn);
	}
	if (!isclient) {
		get_packet(conn);
		send_packet(conn);
	}
	if (isclient) {
		get_packet(conn);
	}
	Timer timer;
	const int TEST_RUNS = 500;
	for (int i = 0; i < TEST_RUNS; i++) {
		send_packet(conn);
		get_packet(conn);
	}

	printf("Average time was %.2fms\n", timer.get_microseconds()/TEST_RUNS/1000.0);
	lanarts_net_quit();
	return 0;
}

int main(int argc, char** argv) {
	int exit_code = run_unittests();
//	run_client_server_test(argc, argv);
	return exit_code;
}
