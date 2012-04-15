#ifndef __CONNECTION_H_
#define __CONNECTION_H_

#include "packet.h"
#include <vector>

class NetConnection {
public:
	virtual bool get_next_packet(NetPacket& packet) = 0;
	virtual void broadcast_packet(const NetPacket& packet) = 0;
	virtual void join() = 0;
	virtual int get_peer_id() = 0;
	virtual int get_number_peers() = 0;

	virtual ~NetConnection(){}
};


NetConnection* create_server_connection(int port);
NetConnection* create_dummy_connection();
NetConnection* create_client_connection(const char* host, const char* port);

#endif

