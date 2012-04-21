#include "ClientNetConnection.h"
#include <boost/bind.hpp>

using namespace asio::ip;

void client_connect_handler(ClientNetConnection* cnc,
		const asio::error_code& error,
		tcp::resolver::iterator endpoint_iterator) {
	SocketStream* s = &cnc->socket_stream();
	if (!error) {
		NetPacket packet(cnc->get_peer_id());
		s->send_packet(packet);
		cnc->set_connected();

		asio::async_read(
				s->get_socket(),
				asio::buffer(s->last_message().data, NetPacket::HEADER_LEN),
				boost::bind(socketstream_read_header_handler, s,
						asio::placeholders::error));
	} else if (endpoint_iterator != tcp::resolver::iterator()) {
		s->get_socket().close();
		tcp::endpoint endpoint = *endpoint_iterator;
		s->get_socket().async_connect(
				endpoint,
				boost::bind(client_connect_handler, cnc,
						asio::placeholders::error, ++endpoint_iterator));
	}
}

ClientNetConnection::ClientNetConnection(const char* host, const char* port) :
		io_service(), stream(io_service) {

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, port);
    tcp::resolver::iterator iterator = resolver.resolve(query);

	asio::ip::tcp::endpoint endpoint = *iterator;
	connected = false;

	stream.get_socket().async_connect(
			endpoint,
			boost::bind(client_connect_handler, this, asio::placeholders::error, ++iterator));

    execution_thread = boost::shared_ptr<asio::thread>(
    		new asio::thread(boost::bind(&asio::io_service::run, &io_service))
    );

}

ClientNetConnection::~ClientNetConnection() {
}

bool ClientNetConnection::get_next_packet(NetPacket & packet) {
	return stream.get_next_packet(packet);
	/*while (true){
		if (packet.packet_type == NetPacket::PACKET_ASSIGN_PEERID){
			peer_id = packet.get_int();
		} else if (packet.packet_type == NetPacket::PACKET_BROADCAST_PEERLISTSIZE){
			number_of_peers = packet.get_int();
		} else break;
		stream.get_next_packet(packet);
	}*/
}

void ClientNetConnection::broadcast_packet(const NetPacket & packet) {
	stream.send_packet(packet);
}

void ClientNetConnection::join() {
	execution_thread->join();
}
