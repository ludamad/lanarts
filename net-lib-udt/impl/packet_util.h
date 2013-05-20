/*
 * packet_util.h:
 *  Simple functions for treating vector as a packet buffer
 */

#ifndef PACKET_UTIL_H_
#define PACKET_UTIL_H_

#include <cstdio>
#include <cstring>
#include <vector>
#include <cassert>

#define LNET_ASSERT(x) assert(x)

typedef std::vector<char> PacketBuffer;

static const int HEADER_SIZE = sizeof(int) * 3;

inline void prepare_packet(PacketBuffer& packet, const char* msg, int len,
		int receiver, int sender = 0) {
	packet.clear();
	packet.reserve(len + HEADER_SIZE);
	packet.insert(packet.end(), (char*) &receiver,
			((char*) &receiver) + sizeof(int));
	packet.insert(packet.end(), (char*) &sender,
			((char*) &sender) + sizeof(int));
	packet.insert(packet.end(), (char*) &len, ((char*) &len) + sizeof(int));
	packet.insert(packet.end(), msg, msg + len);
//	printf("prepping with %d %d %d\n", receiver, sender, len);
}

inline void set_packet_sender(PacketBuffer& packet, int sender = 0) {
	memcpy(&packet.at(sizeof(int)), &sender, sizeof(int));

}
inline void send_packet(UDTSOCKET socket, PacketBuffer& packet) {
	if (UDT::send(socket, &packet[0], packet.size(), 0) == UDT::ERROR) {
		printf("Error sending packet!\n");
	}
}

/* Ensure we can read the header even if we somehow get less than HEADER_SIZE bytes */
inline bool read_n_bytes(UDTSOCKET socket, PacketBuffer& packet, int nbytes) {
	int nread = 0;
	while (nread < nbytes) {
		int recv = UDT::recv(socket, &packet[nread], nbytes, 0);
		if (recv == UDT::ERROR) {
			fprintf(stderr,
					"Connection severed, read_n_bytes got error message:\n\t%s\n",
					UDT::getlasterror().getErrorMessage());
			fflush(stderr);
			return false;
		}
		nread += recv;
	}
	return true;
}

inline bool receive_packet(UDTSOCKET socket, PacketBuffer& packet,
		receiver_t& receiver, receiver_t& sender) {
	packet.resize(HEADER_SIZE);

	if (!read_n_bytes(socket, packet, HEADER_SIZE)) {
		return false;
	}

	receiver = *(int*) &packet[0];
	sender = *(int*) &packet[sizeof(int)];
	int size = *(int*) &packet[sizeof(int) * 2];

//		printf("recving with %d %d %d\n", receiver, sender, size);
	int body_read = 0;
	packet.resize(size + HEADER_SIZE);

	while (size > body_read) {
		int needed = size - body_read;
		int nbody = UDT::recv(socket, &packet[body_read + HEADER_SIZE], needed,
				0);
		if (nbody == UDT::ERROR) {
			fprintf(stderr,
					"Connection severed, receive_packet got error message:\n\t%s\n",
					UDT::getlasterror().getErrorMessage());
			fflush(stderr);
			return false;
		}
		body_read += nbody;
	}
	return true;
}

#endif /* PACKET_UTIL_H_ */
