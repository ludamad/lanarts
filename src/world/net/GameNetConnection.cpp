
#include "GameNetConnection.h"

GameNetConnection::GameNetConnection(NetConnection *connect)
	: connect(connect) {
}

GameNetConnection::~GameNetConnection() {
	delete connect;
}

void GameNetConnection::send_and_sync(const NetPacket & packet, std::vector<NetPacket>& received) {
	if (!connect) return;
	NetPacket local;
	connect->broadcast_packet(packet);
	for (;;) {
		if (connect->get_next_packet(local)) {

		}

	}
}

