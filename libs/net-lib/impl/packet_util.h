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

inline int get_epacket_sender(ENetPacket* epacket) {
	int sender;
	memcpy(&sender, epacket->data + sizeof(int), sizeof(int));
	return sender;
}
inline void set_epacket_sender(ENetPacket* epacket, int sender) {
	memcpy(epacket->data + sizeof(int), &sender, sizeof(int));
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

inline void copy_packet_to_buffer(ENetPacket* epacket, PacketBuffer& packet,
		receiver_t& receiver, receiver_t& sender) {
	packet.assign(epacket->data, epacket->data + epacket->dataLength);

	receiver = *(int*) &packet[0];
	sender = *(int*) &packet[sizeof(int)];
}


#endif /* PACKET_UTIL_H_ */
