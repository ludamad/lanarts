#include "GameNetConnection.h"
#include "../GameState.h"

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
	if (connect)
		connect->broadcast_packet(packet, send_to_new);
}

void GameNetConnection::finalize_connections() {
	if (connect)
		connect->finalize_connections();
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
			printf("Conflicting value theirs %d vs ours %d for sender %d\n",
					theirvalue, value, i);
			return false;
		}
	}
	return true;
}
void GameNetConnection::wait_for_packet(NetPacket & packet) {
	if (!connect)
		return;
	while (!connect->get_next_packet(packet)) {
		//Continue until condition is true
	}
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

