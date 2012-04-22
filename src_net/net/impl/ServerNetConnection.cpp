#include "ServerNetConnection.h"
#include <boost/bind.hpp>

typedef boost::shared_ptr<SocketStream> stream_ptr;

void ServerNetConnection::get_peer_packets(std::vector<NetPacket> & packets){
	packets.resize(get_number_peers());
	for (int i = 0; i < packets.size(); i++){

	}
}

void ServerNetConnection::async_read(SocketStream* ss){
	asio::async_read(
			ss->get_socket(),
			asio::buffer(ss->last_message().data, NetPacket::HEADER_LEN),
			boost::bind(socketstream_read_header_handler, ss,
					asio::placeholders::error));
}
void ServerNetConnection::accept_handler(SocketStream* ss, const asio::error_code& error) {
	if (!error) {
		if (ss){
			streamlock.lock();
			streams.push_back(stream_ptr(ss));
			streamlock.unlock();

			printf("connection accepted\n");
			asio::async_read(
					ss->get_socket(),
					asio::buffer(ss->last_message().data, NetPacket::HEADER_LEN),
					boost::bind(socketstream_read_header_handler, ss,
							asio::placeholders::error));
		}
		ss = new SocketStream(io_service);
//		peer_id = ss->get_peer_id();
		acceptor.async_accept(
			ss->get_socket(),
			boost::bind(&ServerNetConnection::accept_handler, this, ss, asio::placeholders::error));
	}
}

static void wrapped_run(asio::io_service* ios){
	try {
		ios->run();
	} catch (const std::exception& e){
		printf("type=%d\n", typeid(e).name());
		printf("%s\n", e.what());
	}
	printf("io_service::run completed!\n");
}

ServerNetConnection::ServerNetConnection(int port) :
		io_service(), endpoint(asio::ip::tcp::v4(), port), acceptor(io_service,endpoint) {

	io_service.post(
    		boost::bind(&ServerNetConnection::accept_handler, this, (SocketStream*)NULL, asio::error_code())
    );
//	accept_handler(NULL, asio::error_code());
	execution_thread = boost::shared_ptr<asio::thread>(
    	new asio::thread(boost::bind(&wrapped_run, &io_service))
    );

	bool done = false;
	while (!done){
	streamlock.lock();
	done = !streams.empty();
	streamlock.unlock();
	}
}


ServerNetConnection::~ServerNetConnection() {
}

void ServerNetConnection::assign_peerid(SocketStream* stream, int peerid){
	NetPacket packet(get_peer_id());
	packet.packet_type = NetPacket::PACKET_ASSIGN_PEERID;
	packet.add_int(peerid);
	stream->send_packet(packet);
}
bool ServerNetConnection::get_next_packet(NetPacket & packet) {
	bool found = false;

	streamlock.lock();
	std::vector< stream_ptr > s = streams;
	streamlock.unlock();

	/*
	for (int i = 0; i < s.size() && !found; i++){
		SocketStream* ss = s[i].get();
		boost::mutex& m = ss->get_mutex();
		//We try to determine the status without a lock, should never be 0 when non-empty
		if (ss->rmessages().size() != 0){
			m.lock();
			do {
				packet = ss->rmessages().front();
				ss->rmessages().pop_front();
				if (packet.packet_type == NetPacket::PACKET_HELLO){
					assign_peerid(ss, i+1);
				}
			} while (packet.packet_type == NetPacket::PACKET_HELLO);
			m.unlock();
			found = true;
		}
	}*/

	for (int i = 0; i < s.size() && !found; i++){
		SocketStream* ss = s[i].get();
		boost::mutex& m = ss->get_rmutex();
		//We try to determine the status without a lock, should never be 0 when non-empty
		m.lock();
		if (ss->rmessages().size() != 0){
			packet = ss->rmessages().front();
			ss->rmessages().pop_front();
			found = true;
		}
		m.unlock();
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

