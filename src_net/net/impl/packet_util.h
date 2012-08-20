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
//	printf("prepping with %d %d %d\n", receiver, sender, len);
}

inline void set_packet_sender(PacketBuffer& packet, int sender = 0) {
	memcpy(&packet.at(sizeof(int)), &sender, sizeof(int));

}
inline void send_packet(TCPsocket socket, PacketBuffer& packet) {
	SDLNet_TCP_Send(socket, &packet[0], packet.size());
}

/* Ensure we can read the header even if we somehow get less than HEADER_SIZE bytes */
inline void read_n_bytes(TCPsocket socket, PacketBuffer& packet, int nbytes) {
	int nread = 0;
	while (nread < nbytes) {
		int recv = SDLNet_TCP_Recv(socket, &packet[nread], nbytes);
		if (recv < 0) {
			fprintf(
					stderr,
					"Connection severed, read_n_bytes got error message:\n\t%s\n",
					SDLNet_GetError());
			exit(0);
		}
		nread += recv;
	}
}

inline void receive_packet(TCPsocket socket, PacketBuffer& packet,
		receiver_t& receiver, receiver_t& sender) {
	packet.resize(HEADER_SIZE);

	read_n_bytes(socket, packet, HEADER_SIZE);
	receiver = *(int*)&packet[0];
	sender = *(int*)&packet[sizeof(int)];
	int size = *(int*)&packet[sizeof(int) * 2];

//		printf("recving with %d %d %d\n", receiver, sender, size);
	int body_read = 0;
	packet.resize(size + HEADER_SIZE);

	while (size > body_read) {
		int needed = size - body_read;
		int nbody = SDLNet_TCP_Recv(socket, &packet[body_read + HEADER_SIZE],
				needed);
		if (nbody < 0) {
			fprintf(
					stderr,
					"Connection severed, receive_packet got error message:\n\t%s\n",
					SDLNet_GetError());
			exit(0);
		}
		body_read += nbody;
	}
}

#endif /* PACKET_UTIL_H_ */
