/*
 * packet_util.h:
 *  Simple functions for treating vector as a packet buffer
 */

#ifndef PACKET_UTIL_H_
#define PACKET_UTIL_H_

#include <cstdio>
#include <SDL_net.h>
#include <vector>

typedef std::vector<char> PacketBuffer;

const int HEADER_SIZE = sizeof(int) * 2;

inline void prepare_packet(PacketBuffer& packet, const char* msg, int len,
		int receiver) {
	packet.clear();
	packet.reserve(len + HEADER_SIZE);
	packet.insert(packet.end(), (char*)&receiver,
			((char*)&receiver) + sizeof(int));
	packet.insert(packet.end(), (char*)&len, ((char*)&len) + sizeof(int));
	packet.insert(packet.end(), msg, msg + len);
}

inline void send_packet(TCPsocket socket, PacketBuffer& packet) {
	SDLNet_TCP_Send(socket, &packet[0], packet.size());
}

inline receiver_t receive_packet(TCPsocket socket, PacketBuffer& packet) {
	packet.resize(HEADER_SIZE);

	int nread = SDLNet_TCP_Recv(socket, &packet[0], HEADER_SIZE);
	if (nread < HEADER_SIZE) {
		//TODO: Error properly
		fprintf(stderr, "Error while reading packet header!\n", nread);
		return -2;
	} else {
		int receiver = *(int*)&packet[0];
		int size = *(int*)&packet[sizeof(int)];
		int body_read = nread - HEADER_SIZE;
		packet.resize(size + HEADER_SIZE);

		while (size > body_read) {
			int needed = size - body_read;
			int nbody = SDLNet_TCP_Recv(socket,
					&packet[body_read + HEADER_SIZE], needed);
			if (nbody < 0) {
				//TODO: Error properly
				fprintf(stderr, "Error while reading packet body!\n", nbody);
				return 0;
			}
			body_read += nbody;
		}
		return receiver;
	}
}

#endif /* PACKET_UTIL_H_ */
