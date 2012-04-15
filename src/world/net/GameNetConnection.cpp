#include "GameNetConnection.h"

GameNetConnection::GameNetConnection(NetConnection *connect) :
		connect(connect) {
}

GameNetConnection::~GameNetConnection() {
	delete connect;
}

void GameNetConnection::add_peer_id(int peer_id) {
	peer_ids.push_back( peer_id );
}

void GameNetConnection::wait_for_packet(NetPacket & packet) {
	if (!connect) return;
	while (!connect->get_next_packet(packet)) {
		//Continue until condition is true
	}
}

void GameNetConnection::send_and_sync(const NetPacket & packet,
		std::vector<NetPacket>& received) {
	if (!connect)
		return;
	NetPacket local;
	connect->broadcast_packet(packet);
	for (;;) {
		wait_for_packet(local);
	//	local.get_int()
	}
}

