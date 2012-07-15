/* GameNetConnection.cpp:
 * 	Utility class for managing communication with other players
 * 	If no network communication is used, the methods of this class
 * 	return trivial results.
 * 	Provides useful behaviour on top of the src_net library.
 */

#include "../world/GameState.h"

#include "GameNetConnection.h"

GameNetConnection::GameNetConnection(NetConnection *connect) :
		connect(connect) {
}

GameNetConnection::~GameNetConnection() {
	delete connect;
}

void GameNetConnection::add_peer_id(int peer_id) {
	peer_ids.push_back(peer_id);
}

void GameNetConnection::broadcast_packet(const NetPacket & packet,
		bool send_to_new) {
	if (connect) {
		connect->broadcast_packet(packet, send_to_new);
	}
}

void GameNetConnection::finalize_connections() {
	if (connect) {
		connect->finalize_connections();
	}
}

void GameNetConnection::wait_for_packet(NetPacket& packet, int packettype) {
	if (!connect) {
		return;
	}
	while (!connect->get_next_packet(packet)) {
		//Continue until condition is true
	}
}

bool GameNetConnection::get_next_packet(NetPacket& packet, packet_t type) {
	if (!connect) {
		return false;
	}
	return connect->get_next_packet(packet, type);
}

bool GameNetConnection::check_integrity(GameState* gs, int value) {
	NetPacket packet;
	packet.add_int(value);
	packet.encode_header();
	std::vector<NetPacket> packets;
	gs->net_connection().send_and_sync(packet, packets, false);
	for (int i = 0; i < packets.size(); i++) {
		NetPacket& p = packets[i];
		int theirvalue = p.get_int();
		if (theirvalue != value) {
			fprintf(
					stderr,
					"Conflicting value theirs 0x%X vs ours 0x%X for sender %d\n",
					theirvalue, value, i);
			fflush(stderr);
			return false;
		}
	}
	return true;
}

void GameNetConnection::send_and_sync(const NetPacket & packet,
		std::vector<NetPacket>& received, bool send_to_new) {
	if (!connect)
		return;
	NetPacket local;
	connect->broadcast_packet(packet, send_to_new);
	wait_for_packet(local);
	received.push_back(local);
}

