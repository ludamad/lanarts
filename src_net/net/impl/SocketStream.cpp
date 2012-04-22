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
//		if (ss->last_message().body_length > 0){
			asio::async_read(
					ss->get_socket(),
					asio::buffer(ss->last_message().body(),
							ss->last_message().body_length),
					boost::bind(socketstream_read_body_handler, ss,
							asio::placeholders::error));
//		} else {
//			asio::async_read(
//					ss->get_socket(),
//					asio::buffer(ss->last_message().data, NetPacket::HEADER_LEN),
//					boost::bind(&socketstream_read_header_handler, ss,
//							asio::placeholders::error));
//		}
	} else {
//		socketstream_do_close(ss);
	}
}

void socketstream_read_body_handler(SocketStream* ss,
		const asio::error_code& error) {
	if (!error) {
		ss->last_message().decode_header();
		ss->get_rmutex().lock();
		ss->rmessages().push_back(boost::shared_ptr<NetPacket>(new NetPacket(ss->last_message())));
		ss->get_rmutex().unlock();

		static int msg = 0;
//		printf("Read queue size '%d' msg#'%d' \n", ss->rmessages().size(), ++msg);
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
		const asio::error_code& error, bool pop) {
	if (!error) {
		static int msg = 0;
//		printf("Write queue size '%d' msg#'%d' \n", ss->wmessages().size(), ++msg);
		ss->get_wmutex().lock();
		if (pop)
			ss->wmessages().pop_front();
		if (!ss->wmessages().empty()){
			asio::async_write(
					ss->get_socket(),
					asio::buffer(ss->wmessages().front()->data, ss->wmessages().front()->length()),
					boost::bind(socketstream_write_handler, ss,
							asio::placeholders::error, true));
		}
		ss->get_wmutex().unlock();
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

	rmutex.lock();
	if (reading_msgs.size() != 0){
		packet = *reading_msgs.front().get();
		reading_msgs.pop_front();
		rmutex.unlock();
		return true;
	}
	rmutex.unlock();

	return false;
}


void SocketStream::send_packet(const NetPacket & packet) {
	if (closed) {
		printf("Pools closed due to AIDS\n!");
		return;
	}

	wmutex.lock();
		bool write_in_progress = !writing_msgs.empty();
		writing_msgs.push_back(boost::shared_ptr<NetPacket>(new NetPacket(packet)));
		if (!write_in_progress){
			io_service.post(boost::bind(socketstream_write_handler, this,
					asio::error_code(), false));
		}
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
