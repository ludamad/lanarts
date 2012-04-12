#include "ServerNetConnection.h"
#include <boost/bind.hpp>

typedef boost::shared_ptr<SocketStream> stream_ptr;

void ServerNetConnection::accept_handler(SocketStream* ss, const asio::error_code& error) {
	if (!error) {
		if (ss){
			streamlock.lock();
			streams.push_back(stream_ptr(ss));
			streamlock.unlock();

			asio::async_read(
					ss->get_socket(),
					asio::buffer(ss->last_message().data, NetPacket::HEADER_LEN),
					boost::bind(socketstream_read_header_handler, streams.back().get(),
							asio::placeholders::error));
		}
		ss = new SocketStream(io_service);
		acceptor.async_accept(
			ss->get_socket(),
			boost::bind(&ServerNetConnection::accept_handler, this, ss, asio::placeholders::error));
	}
}
ServerNetConnection::ServerNetConnection(int port) :
		io_service(), endpoint(asio::ip::tcp::v4(), port), acceptor(io_service,endpoint) {

	io_service.post(
    		boost::bind(&ServerNetConnection::accept_handler, this, (SocketStream*)NULL, asio::error_code())
    );

	execution_thread = boost::shared_ptr<asio::thread>(
    	new asio::thread(boost::bind(&asio::io_service::run, &io_service))
    );
}


ServerNetConnection::~ServerNetConnection() {
}

bool ServerNetConnection::get_next_packet(NetPacket & packet) {
	bool found = false;

	streamlock.lock();
	std::vector< stream_ptr > s = streams;
	streamlock.unlock();

	for (int i = 0; i < s.size() && !found; i++){
		SocketStream* ss = s[i].get();
		boost::mutex& m = ss->get_mutex();
		//We try to determine the status without a lock, should never be 0 when non-empty
		if (ss->rmessages().size() != 0){
			m.lock();
			packet = ss->rmessages().front();
			ss->rmessages().pop_front();
			m.unlock();
			found = true;
		}
	}
	return found;
}

void ServerNetConnection::broadcast_packet(const NetPacket & packet) {
	streamlock.lock();
	for (int i = 0; i < streams.size(); i++){
		streams[i]->send_packet(packet);
	}
	streamlock.unlock();
}

void ServerNetConnection::join(){
	execution_thread->join();
}

