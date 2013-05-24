/*
 * packet_util.h:
 *  Simple functions for treating vector as a packet buffer
 */

#ifndef PACKET_UTIL_H_
#define PACKET_UTIL_H_

#include <cstdio>
#include <cstring>
#include <enet/enet.h>
#include <vector>

typedef std::vector<char> PacketBuffer;

static const int HEADER_SIZE = sizeof(int) * 2;

inline void prepare_packet(PacketBuffer& packet, const char* msg, int len,
		int receiver, int sender = 0) {
	packet.clear();
	packet.reserve(len + HEADER_SIZE);
	packet.insert(packet.end(), (char*)&receiver,
			((char*)&receiver) + sizeof(int));
	packet.insert(packet.end(), (char*)&sender, ((char*)&sender) + sizeof(int));
	packet.insert(packet.end(), msg, msg + len);
}

inline void set_packet_sender(PacketBuffer& packet, int sender = 0) {
	memcpy(&packet.at(sizeof(int)), &sender, sizeof(int));
}
inline int get_epacket_sender(ENetPacket* epacket) {
	int sender;
	memcpy(&sender, epacket->data + sizeof(int), sizeof(int));
	return sender;
}

inline ENetPacket* make_epacket(PacketBuffer& packet, bool reliable) {
	return enet_packet_create((void*)&packet[0], packet.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
}

inline void send_packet(ENetPeer* peer, PacketBuffer& packet, bool reliable = true) {
	enet_peer_send(peer, 0, make_epacket(packet, reliable));
}

inline void broadcast_packet(ENetHost* host, PacketBuffer& packet, bool reliable = true) {
	enet_host_broadcast(host, 0, make_epacket(packet, reliable));
}

/* Note: destroys the enet packet! */
inline void copy_and_release_packet(ENetPacket* epacket, PacketBuffer& packet,
		receiver_t& receiver, receiver_t& sender) {
	packet.assign(epacket->data, epacket->data + epacket->dataLength);

	receiver = *(int*) &packet[0];
	sender = *(int*) &packet[sizeof(int)];

	enet_packet_destroy(epacket);
}


#endif /* PACKET_UTIL_H_ */
