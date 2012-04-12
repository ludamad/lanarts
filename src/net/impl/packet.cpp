/*
 * packet.cpp
 *
 *  Created on: Apr 8, 2012
 *      Author: 100397561
 */

#include "../packet.h"
#include <cstring>

typedef unsigned long long ullong;

NetPacket::NetPacket() :
		body_length(0) {
}

size_t NetPacket::length() const {
	return sizeof(short) + body_length;
}

char *NetPacket::body() {
	return data + sizeof(short);
}

bool NetPacket::decode_header() {
	body_length = *((short *) ((((data)))));
	if (body_length > MAX_PACKET_SIZE) {
		body_length = 0;
		return false;
	}
	return true;
}

void NetPacket::encode_header() {
	*(short *) data = body_length;
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
