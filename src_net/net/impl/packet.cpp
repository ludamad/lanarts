/*
 * packet.cpp
 *
 *  Created on: Apr 8, 2012
 *      Author: 100397561
 */

#include "../packet.h"
#include <cstring>

typedef long long llong;

NetPacket::NetPacket(int packet_origin) :
		body_length(0), packet_type(PACKET_MSG), packet_origin(packet_origin) {
}

size_t NetPacket::length() const {
	return HEADER_LEN + body_length;
}

char* NetPacket::body_start() {
	return data + HEADER_LEN;
}
char* NetPacket::body_end() {
	return data + length();
}

bool NetPacket::decode_header() {
	int* idata = (int*) data;
	body_length = idata[0];
	if (body_length > MAX_PACKET_SIZE) {
		printf("TOO LARGE size = %d\n", body_length);
		body_length = 0;
		return false;
	}
	packet_type = idata[1];
	packet_origin = idata[2];

	return true;
}

void NetPacket::encode_header() {
	int* idata = (int*) data;
	idata[0] = body_length;
	idata[1] = packet_type;
	idata[2] = packet_origin;
}

void NetPacket::add_int(long val) {
	llong l = val;
	memcpy(body_end(), &l, sizeof(llong));
	body_length += sizeof(llong);
}

void NetPacket::add_double(double val) {
	memcpy(body_end(), &val, sizeof(double));
	body_length += sizeof(double);
}

long NetPacket::get_int() {
	body_length -= sizeof(llong);
	return *(llong*) body_end();
}

double NetPacket::get_double() {
	body_length -= sizeof(double);
	return *(double*) body_end();
}
