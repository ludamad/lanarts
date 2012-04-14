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
	std::deque<NetPacket>& rmessages()  { return reading_msgs;}
	std::deque<NetPacket>& wmessages()  { return writing_msgs;}
	asio::ip::tcp::socket& get_socket(){ return socket; }
	boost::mutex& get_mutex(){ return mutex;}
	NetPacket& last_message() { return read_message; }
	bool& is_closed(){ return closed; }

private:
	bool closed;
	boost::mutex mutex;
	NetPacket read_message;
	asio::io_service& io_service;
	asio::ip::tcp::socket socket;
	std::deque<NetPacket> reading_msgs, writing_msgs;
};

#endif /* SOCKETSTREAM_H_ */
