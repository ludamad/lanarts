#include <cstring>
#include <cstdio>

#include <vector>
#include <string>

#include <net/lanarts_net.h>

#include <lcommon/unittest.h>

SUITE(net_unit_tests) {

	const int TEST_PORT = 6112;
	static int callback_calls;

	/* Enables thread-safety in resource deallocation */
	struct CleanNetConnection {
		CleanNetConnection(NetConnection* conn = NULL) :
				conn(conn) {
		}
		~CleanNetConnection() {
			delete conn;
		}
		NetConnection* operator->() {
			return conn;
		}
		NetConnection* conn;
	};

	static void assert_equal_msg(receiver_t sender, void* teststr,
			const char* msg, size_t len) {
		const char* tester = (char*) teststr;
		callback_calls++;
		UNIT_TEST_ASSERT(strncmp(tester, msg, len) == 0);
	}

	TEST(test_simple_client_server_msg) {
		const char* msg = "Hello World!";
		callback_calls = 0;

		CleanNetConnection server = create_server_connection(TEST_PORT);
		CleanNetConnection client = create_client_connection("localhost",
				TEST_PORT);

		server->initialize_connection();
		client->initialize_connection();

		server->poll(NULL);

		server->send_message(msg, strlen(msg), NetConnection::ALL_RECEIVERS);
		client->send_message(msg, strlen(msg), NetConnection::ALL_RECEIVERS);

		server->poll(assert_equal_msg, (void*) msg);
		client->poll(assert_equal_msg, (void*) msg);

		UNIT_TEST_ASSERT(callback_calls == 2);
	}

	static void test_multiple_clients() {
		const char* msgsrv = "Server Message";
		const char* msgclient = "Client Message";
		callback_calls = 0;

		CleanNetConnection server = create_server_connection(TEST_PORT);
		CleanNetConnection client1 = create_client_connection("localhost",
				TEST_PORT);
		CleanNetConnection client2 = create_client_connection("localhost",
				TEST_PORT);

		server->initialize_connection();
		client1->initialize_connection();
		client2->initialize_connection();

		server->poll(NULL);

		server->send_message(msgsrv, strlen(msgsrv),
				NetConnection::ALL_RECEIVERS);
		client1->send_message(msgclient, strlen(msgclient),
				NetConnection::SERVER_RECEIVER);

		server->poll(assert_equal_msg, (void*) msgclient);
		client1->poll(assert_equal_msg, (void*) msgsrv);
		client2->poll(assert_equal_msg, (void*) msgsrv);

		UNIT_TEST_ASSERT(callback_calls == 3);
	}

	static void append_to_string_vector(receiver_t sender, void* strvec,
			const char* msg, size_t len) {
		std::vector<std::string>* strs = (std::vector<std::string>*) strvec;
		strs->push_back(std::string(msg, len));
	}

	TEST(test_multiple_messages) {
		const char* msgs[] = { "Message 1", "Message 2", "Message 3" };
		const int msg_n = 3;

		callback_calls = 0;

		CleanNetConnection server = create_server_connection(TEST_PORT);
		CleanNetConnection client = create_client_connection("localhost",
				TEST_PORT);

		server->initialize_connection();
		client->initialize_connection();

		server->poll(NULL);

		for (int i = 0; i < msg_n; i++) {
			server->send_message(msgs[i], strlen(msgs[i]),
					NetConnection::ALL_RECEIVERS);
		}

		for (int i = 0; i < msg_n; i++) {
			client->send_message(msgs[i], strlen(msgs[i]),
					NetConnection::ALL_RECEIVERS);
		}

		std::vector<std::string> servermsglist;
		std::vector<std::string> clientmsglist;

		server->poll(append_to_string_vector, (void*) &servermsglist);
		client->poll(append_to_string_vector, (void*) &clientmsglist);

		UNIT_TEST_ASSERT(servermsglist.size() == msg_n);
		UNIT_TEST_ASSERT(clientmsglist.size() == msg_n);

		for (int i = 0; i < msg_n; i++) {
			UNIT_TEST_ASSERT(servermsglist.at(i) == msgs[i]);
			UNIT_TEST_ASSERT(clientmsglist.at(i) == msgs[i]);
		}
	}

}
