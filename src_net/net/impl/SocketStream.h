/*
 * SocketStream.h
 *
 *  Created on: Apr 4, 2012
 *      Author: 100397561
 */

#ifndef SOCKETSTREAM_H_
#define SOCKETSTREAM_H_

#include "../connection.h"
#include <asio.hpp>
#include <deque>
#include <vector>
#include <boost/thread/mutex.hpp>

class SocketStream;
//Handler declarations
void socketstream_do_close(SocketStream* cnc);
void socketstream_read_header_handler(SocketStream* cnc,
		const asio::error_code& error);
void socketstream_read_body_handler(SocketStream* cnc,
		const asio::error_code& error);
void socketstream_write_handler(SocketStream* cnc,
		const asio::error_code& error);

class SocketStream {
public:
	SocketStream(asio::io_service& io_service);

	~SocketStream();

	bool get_next_packet(NetPacket& packet);
	void send_packet(const NetPacket& packet);

	//Use these getters/setters only if you know what you're doing:
	std::list<NetPacket>& rmessages()  { return reading_msgs;}
	std::list<NetPacket>& wmessages()  { return writing_msgs;}
	asio::ip::tcp::socket& get_socket(){ return socket; }
	boost::mutex& get_rmutex(){ return rmutex;}
	boost::mutex& get_wmutex(){ return wmutex;}
	NetPacket& last_message() { return read_message; }
	bool& is_closed(){ return closed; }
//	int get_peer_id();
	//Returns a max(peer)-size list
	void get_connections(std::vector<NetPacket>& packets);

private:
	bool closed;
	boost::mutex rmutex, wmutex;
	NetPacket read_message;
	asio::io_service& io_service;
	asio::ip::tcp::socket socket;
	std::vector<NetPacket> peerpackets;
	std::list<NetPacket> reading_msgs, writing_msgs;
};

#endif /* SOCKETSTREAM_H_ */
