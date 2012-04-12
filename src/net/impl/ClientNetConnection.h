/*
 * ClientNetConnection.h
 *
 *  Created on: Apr 3, 2012
 *      Author: 100397561
 */

#ifndef CLIENTNETCONNECTION_H_
#define CLIENTNETCONNECTION_H_

#include "../connection.h"
#include <asio.hpp>
#include "SocketStream.h"
#include <boost/shared_ptr.hpp>

class ClientNetConnection : public NetConnection {
public:
	ClientNetConnection(const char* host, const char* port);

	virtual ~ClientNetConnection();

	virtual bool get_next_packet(NetPacket& packet);
	virtual void broadcast_packet(const NetPacket& packet);

	SocketStream& socket_stream() { return stream; }
	virtual void join();

private:
	boost::shared_ptr<asio::thread> execution_thread;
	asio::io_service io_service;
	SocketStream stream;
};

#endif /* CLIENTNETCONNECTION_H_ */
