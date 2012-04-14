#ifndef PACKET_H_
#define PACKET_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>

struct NetPacket {
	enum {
		HEADER_LEN = sizeof(short), MAX_PACKET_SIZE = 512
	};

	NetPacket();
	size_t length() const;
    char *body();
    bool decode_header();
    void encode_header();

	void add_int(long val);
	void add_double(double val);
	long get_int();
	double get_double();

	template<typename T>
	void add(const T& t) {
		memcpy(body(), &t, sizeof(T));
		body_length += sizeof(T);
	}
	template<typename T>
	void get(T& t) {
		body_length -= sizeof(T);
		memcpy(&t, body(), sizeof(T));
	}

	char data[sizeof(short) + MAX_PACKET_SIZE];
	size_t body_length;
};

#endif /* PACKET_H_ */
