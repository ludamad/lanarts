#include "ClientNetConnection.h"
#include <boost/bind.hpp>

using namespace asio::ip;


void socketstream_do_close(SocketStream* ss) {
	if (!ss->is_closed()){
		ss->get_socket().close();
		ss->is_closed() = true;
	}
}

void socketstream_read_header_handler(SocketStream* ss,
		const asio::error_code& error) {
	if (!error && ss->last_message().decode_header()) {
		asio::async_read(
				ss->get_socket(),
				asio::buffer(ss->last_message().body(),
						ss->last_message().body_length),
				boost::bind(socketstream_read_body_handler, ss,
						asio::placeholders::error));
	} else {
		socketstream_do_close(ss);
	}
}

void socketstream_read_body_handler(SocketStream* ss,
		const asio::error_code& error) {
	if (!error) {
		ss->get_mutex().lock();
		ss->rmessages().push_back(ss->last_message());
		ss->get_mutex().unlock();

		asio::async_read(
				ss->get_socket(),
				asio::buffer(ss->last_message().data, NetPacket::HEADER_LEN),
				boost::bind(&socketstream_read_header_handler, ss,
						asio::placeholders::error));

	} else {
		socketstream_do_close(ss);
	}
}

void socketstream_write_handler(SocketStream* ss,
		const asio::error_code& error) {
	if (!error) {
		ss->get_mutex().lock();
		ss->wmessages().pop_front();
		ss->get_mutex().unlock();

		if (!ss->wmessages().empty()) {
			NetPacket& next = ss->wmessages().front();
			asio::async_write(
					ss->get_socket(),
					asio::buffer(next.data, next.length()),
					boost::bind(socketstream_write_handler, ss,
							asio::placeholders::error));
		}
	} else {
		socketstream_do_close(ss);
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
		mutex.lock();
		packet = reading_msgs.front();
		reading_msgs.pop_front();
		mutex.unlock();
		return true;
	}

	return false;
}

void SocketStream::send_packet(const NetPacket & packet) {
	if (closed) return;

	mutex.lock();
	bool write_in_progress = !writing_msgs.empty();
	writing_msgs.push_front(packet);
	mutex.unlock();
    if (!write_in_progress){
		asio::async_write(
				socket,
				asio::buffer(writing_msgs.front().data,
						writing_msgs.front().length()),
				boost::bind(socketstream_write_handler, this,
						asio::placeholders::error));
    }
}
