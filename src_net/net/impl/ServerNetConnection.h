/*
 * ServerNetConnection.h
 *
 *  Created on: Apr 3, 2012
 *      Author: 100397561
 */

#ifndef SERVERNETCONNECTION_H_
#define SERVERNETCONNECTION_H_

#include "../connection.h"
#include <asio.hpp>
#include <deque>
#include <boost/shared_ptr.hpp>
#include "SocketStream.h"

class ServerNetConnection : public NetConnection {
public:
	ServerNetConnection(int port);
	virtual ~ServerNetConnection();

	virtual bool get_next_packet(NetPacket& packet);
	virtual void broadcast_packet(const NetPacket& packet);
	virtual int get_peer_id(){ return 0;}
	virtual int get_number_peers(){ return streams.size();}
	virtual void join();

private:
	void assign_peerid(SocketStream* stream, int peerid);
	boost::mutex streamlock;
	boost::shared_ptr<asio::thread> execution_thread;
	void accept_handler(SocketStream* ss, const asio::error_code& error);

	std::vector< boost::shared_ptr<SocketStream> > streams;
	asio::io_service io_service;
	asio::ip::tcp::endpoint endpoint;
	asio::ip::tcp::acceptor acceptor;
};

#endif /* SERVERNETCONNECTION_H_ */
