#include "ClientNetConnection.h"
#include <boost/bind.hpp>
#include <string>

using namespace asio::ip;


void socketstream_do_close(SocketStream* ss) {
	if (!ss->is_closed()){
//		ss->get_socket().close();
//		ss->is_closed() = true;
	}
}

void socketstream_read_header_handler(SocketStream* ss,
		const asio::error_code& error) {
	if (!error && ss->last_message().decode_header()) {
//		printf("reading header of message\n");
		asio::async_read(
				ss->get_socket(),
				asio::buffer(ss->last_message().body(),
						ss->last_message().body_length),
				boost::bind(socketstream_read_body_handler, ss,
						asio::placeholders::error));
	} else {
//		socketstream_do_close(ss);
	}
}

void socketstream_read_body_handler(SocketStream* ss,
		const asio::error_code& error) {
	if (!error) {
		ss->last_message().decode_header();
		ss->get_rmutex().lock();
		ss->rmessages().push_back(ss->last_message());
		ss->get_rmutex().unlock();

		static int msg = 0;
//		printf("Reading message %d\n", ++msg);

		asio::async_read(
				ss->get_socket(),
				asio::buffer(ss->last_message().data, NetPacket::HEADER_LEN),
				boost::bind(&socketstream_read_header_handler, ss,
						asio::placeholders::error));

	} else {
//		socketstream_do_close(ss);
	}
}

void socketstream_write_handler(SocketStream* ss,
		const asio::error_code& error) {
	if (!error) {
		static int msg = 0;

//		printf("Writing message %d\n", ++msg);
		if (ss->wmessages().size() == 0)
			return;
		ss->get_wmutex().lock();
		NetPacket next = ss->wmessages().front();
		ss->wmessages().pop_front();
		ss->get_wmutex().unlock();
		asio::async_write(
				ss->get_socket(),
				asio::buffer(next.data, next.length()),
				boost::bind(socketstream_write_handler, ss,
						asio::placeholders::error));
	} else {
//		socketstream_do_close(ss);
	}
}

SocketStream::SocketStream(asio::io_service & io_service) :
		io_service(io_service), socket(io_service) {
	closed = false;
}

SocketStream::~SocketStream() {
	socketstream_do_close(this);
}

bool SocketStream::get_next_packet(NetPacket & packet) {
	if (closed) return false;

	if (reading_msgs.size() != 0){
		rmutex.lock();
		packet = reading_msgs.front();
		reading_msgs.pop_front();
		rmutex.unlock();
		return true;
	}

	return false;
}

void SocketStream::send_packet(const NetPacket & packet) {
	if (closed) {
		printf("Pools closed due to AIDS\n!");
		return;
	}

	wmutex.lock();
	bool write_in_progress = !writing_msgs.empty();
	asio::write(socket,
					asio::buffer(writing_msgs.front().data,
							writing_msgs.front().length()));
//    if (!write_in_progress){
//		asio::async_write(
//				socket,
//				asio::buffer(writing_msgs.front().data,
//						writing_msgs.front().length()),
//				boost::bind(socketstream_write_handler, this,
//						asio::placeholders::error));
//    } else
//    	writing_msgs.push_front(packet);
	wmutex.unlock();
//    }
}

//static unsigned int to_ip_number(const char* ipString){
//	unsigned char bytes[4] = {0,0,0,0};
//	for (int i = 0; i < 4; i++){
//		bytes[i] = atoi(ipString);
//		while (*ipString != 0 && *ipString != '.')
//			++ipString;
//	}
//	return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
//}
//int SocketStream::get_peer_id(){
//	asio::ip::address remote_ad = socket.remote_endpoint().address();
//	std::string s = remote_ad.to_string();
//	return to_ip_number(s.c_str());
//}
