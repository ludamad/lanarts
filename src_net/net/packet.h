#ifndef PACKET_H_
#define PACKET_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>


struct NetPacket {
	enum {
		HEADER_LEN = sizeof(int)*3, MAX_PACKET_SIZE = 512
	};

	/* Messages internal to the networking library use negative indices
	 * This allows a regular enum to be used for the library's user
	 * In general, the library user should not worry about server/client differences */
	enum {
		/*SRV_ASSERT messages give a client information*/
		PACKET_SRV_ASSERT_NUM_PEERIDS = -1,
		PACKET_SRV_ASSERT_PEERID = -2,
		PACKET_SRV_ASSERT_STARTED = -3,
		PACKET_CLIENT_REQUEST_NUM_PEERIDS = -4,
	};

	NetPacket(int packet_origin = 0, int packet_type = 0);
	size_t length() const;
    char* body_start();
    char* body_end();
    bool decode_header();
    void encode_header();

	void add_int(long val);
	void add_str(const char* str, int len);
	void get_str(char* str, int len);
	void add_double(double val);
	long get_int();
	double get_double();

	template<typename T>
	void add(const T& t) {
		memcpy(body_end(), &t, sizeof(T));
		body_length += sizeof(T);
	}
	template<typename T>
	void get(T& t) {
		body_length -= sizeof(T);
		memcpy(&t, body_end(), sizeof(T));
	}

	char data[HEADER_LEN + MAX_PACKET_SIZE];
	size_t body_length;
	int packet_type;
	int packet_origin;
};

#endif /* PACKET_H_ */
