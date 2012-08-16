/*
 * packet_util.h:
 *  Simple functions for treating vector as a packet buffer
 */

#ifndef PACKET_UTIL_H_
#define PACKET_UTIL_H_

#include <cstdio>
#include <cstring>
#include <SDL_net.h>
#include <vector>

typedef std::vector<char> PacketBuffer;

static const int HEADER_SIZE = sizeof(int) * 3;

inline void prepare_packet(PacketBuffer& packet, const char* msg, int len,
		int receiver, int sender = 0) {
	packet.clear();
	packet.reserve(len + HEADER_SIZE);
	packet.insert(packet.end(), (char*)&receiver,
			((char*)&receiver) + sizeof(int));
	packet.insert(packet.end(), (char*)&sender, ((char*)&sender) + sizeof(int));
	packet.insert(packet.end(), (char*)&len, ((char*)&len) + sizeof(int));
	packet.insert(packet.end(), msg, msg + len);
}

inline void set_packet_sender(PacketBuffer& packet, int sender = 0) {
	memcpy(&packet.at(sizeof(int)), &sender, sizeof(int));

}
inline void send_packet(TCPsocket socket, PacketBuffer& packet) {
	SDLNet_TCP_Send(socket, &packet[0], packet.size());
}

inline void receive_packet(TCPsocket socket, PacketBuffer& packet,
		receiver_t& receiver, receiver_t& sender) {
	packet.resize(HEADER_SIZE);

	int nread = SDLNet_TCP_Recv(socket, &packet[0], HEADER_SIZE);
	if (nread < HEADER_SIZE) {
		//TODO: Error properly
		fprintf(stderr, "Error while reading packet header!\n", nread);
		receiver = -1;
		sender = -1;
	} else {
		receiver = *(int*)&packet[0];
		receiver = *(int*)&packet[sizeof(int)];
		int size = *(int*)&packet[sizeof(int) * 2];
		int body_read = nread - HEADER_SIZE;
		packet.resize(size + HEADER_SIZE);

		while (size > body_read) {
			int needed = size - body_read;
			int nbody = SDLNet_TCP_Recv(socket,
					&packet[body_read + HEADER_SIZE], needed);
			if (nbody < 0) {
				//TODO: Error properly
				fprintf(stderr, "Error while reading packet body!\n", nbody);
				return;
			}
			body_read += nbody;
		}
	}
}

#endif /* PACKET_UTIL_H_ */
