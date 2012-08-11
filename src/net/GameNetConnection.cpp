/* GameNetConnection.cpp:
 * 	Utility class for managing communication with other players
 * 	If no network communication is used, the methods of this class
 * 	return trivial results.
 * 	Provides useful behaviour on top of the src_net library.
 */

#include <net/lanarts_net.h>

#include "../gamestate/GameState.h"

#include "../serialize/SerializeBuffer.h"

#include "GameNetConnection.h"

GameNetConnection::GameNetConnection(GameState* gs) :
		gs(gs), _connection(NULL) {
	_message_buffer = new SerializeBuffer(SerializeBuffer::plain_buffer());
}

GameNetConnection::~GameNetConnection() {
	delete _connection;
	delete _message_buffer;
}

void gamenetconnection_packet_callback(void* context, const char* msg,
		size_t len) {
	((GameNetConnection*)context);
}

SerializeBuffer& GameNetConnection::grab_cleared_buffer(packet_type type) {
	_message_buffer->clear();
	_message_buffer->write_int(type);
	return *_message_buffer;
}

void GameNetConnection::send_packet(SerializeBuffer& serializer) {
	_connection->send_message(serializer.data(), serializer.size());
}

bool GameNetConnection::check_integrity(GameState* gs, int value) {
	return true;
}

void GameNetConnection::initialize_as_client(const char* host, int port) {
	LANARTS_ASSERT(!_connection);
	_connection = create_client_connection(host, port);
	_connection->initialize_connection();
}

void GameNetConnection::initialize_as_server(int port) {
	LANARTS_ASSERT(!_connection);
	_connection = create_server_connection(port);
	_connection->initialize_connection();
}

void GameNetConnection::set_accepting_connections(bool accept) {
	_connection->set_accepting_connections(accept);
}

void GameNetConnection::receive_packet(const char* msg, size_t len) {
	_message_buffer->clear();
	_message_buffer->write_raw(msg, len);
	packet_type type;
	_message_buffer->read_int(type);
	switch (type) {

	}
}
//TODO: net redo
//
//void GameNetConnection::broadcast_packet(const NetPacket & packet,
//		bool send_to_new) {
//	if (connect) {
//		connect->broadcast_packet(packet, send_to_new);
//	}
//}
//
//void GameNetConnection::finalize_connections() {
//	if (connect) {
//		connect->finalize_connections();
//	}
//}
//
//void GameNetConnection::wait_for_packet(NetPacket& packet, int packettype) {
//	if (!connect) {
//		return;
//	}
//	while (!connect->get_next_packet(packet)) {
//		//Continue until condition is true
//	}
//}
//
//bool GameNetConnection::get_next_packet(NetPacket& packet, packet_t type) {
//	if (!connect) {
//		return false;
//	}
//	return connect->get_next_packet(packet, type);
//}
//
//bool GameNetConnection::check_integrity(GameState* gs, int value) {
//	NetPacket packet;
//	packet.add_int(value);
//	packet.encode_header();
//	std::vector<NetPacket> packets;
//	gs->net_connection().send_and_sync(packet, packets, false);
//	for (int i = 0; i < packets.size(); i++) {
//		NetPacket& p = packets[i];
//		int theirvalue = p.get_int();
//		if (theirvalue != value) {
//			fprintf(
//					stderr,
//					"Conflicting value theirs 0x%X vs ours 0x%X for sender %d\n",
//					theirvalue, value, i);
//			fflush(stderr);
//			return false;
//		}
//	}
//	return true;
//}
//
//void GameNetConnection::send_and_sync(const NetPacket & packet,
//		std::vector<NetPacket>& received, bool send_to_new) {
//	if (!connect)
//		return;
//	NetPacket local;
//	printf("Sending synched packet\n");
//	connect->broadcast_packet(packet, send_to_new);
//	printf("Waiting for synched packet\n");
//
//	wait_for_packet(local);
//	received.push_back(local);
//}

