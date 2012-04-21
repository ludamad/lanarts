/*
 * packet.cpp
 *
 *  Created on: Apr 8, 2012
 *      Author: 100397561
 */

#include "../packet.h"
#include <cstring>

typedef unsigned long long ullong;

NetPacket::NetPacket(int packet_origin) :
		body_length(0), packet_type(PACKET_MSG), packet_origin(packet_origin){
}

size_t NetPacket::length() const {
	return HEADER_LEN + body_length;
}

char *NetPacket::body() {
	return data + HEADER_LEN;
}

bool NetPacket::decode_header() {
	int* idata = (int*)data;
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
	int* idata = (int*)data;
	idata[0] = body_length;
	idata[1] = packet_type;
	idata[2] = packet_origin;
}

void NetPacket::add_int(long val) {
	ullong l = val;
	memcpy(body() + body_length, &l, sizeof(ullong));
	body_length += sizeof(ullong);
}
void NetPacket::add_double(double val) {
	memcpy(body() + body_length, &val, sizeof(double));
	body_length += sizeof(double);
}

long NetPacket::get_int() {
	body_length -= sizeof(ullong);
	return *(ullong*) body();
}
double NetPacket::get_double() {
	body_length -= sizeof(double);
	return *(double*) body();
}
