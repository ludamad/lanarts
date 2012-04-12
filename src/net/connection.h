#ifndef __CONNECTION_H_
#define __CONNECTION_H_

#include "packet.h"

class NetConnection {
public:
	virtual bool get_next_packet(NetPacket& packet) = 0;
	virtual void broadcast_packet(const NetPacket& packet) = 0;
	virtual ~NetConnection(){}
	virtual void join() = 0;
};

NetConnection* create_server_connection(int port);
NetConnection* create_client_connection(const char* host, const char* port);

#endif

