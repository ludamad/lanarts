/*
 * ServerConnection.h:
 *  Represents a connection made by a server to multiple clients
 */

#ifndef SERVERCONNECTION_H_
#define SERVERCONNECTION_H_

#include <vector>

#include <enet/enet.h>

#include "../NetConnection.h"

#include "packet_util.h"

class ServerConnection: public NetConnection {
public:
	ServerConnection(int port, int maximum_connections = 32);
	virtual ~ServerConnection();

	virtual void initialize_connection();

	virtual bool poll(packet_recv_callback message_handler,
			void* context = NULL, int timeout = 0);
	virtual void set_accepting_connections(bool accept);
	virtual void send_message(const char* msg, int len, receiver_t receiver =
			ALL_RECEIVERS);

	const std::vector<ENetPeer*>& get_socket_list() {
		return _socket_list;
	}
private:
	void _send_message(PacketBuffer& packet, receiver_t receiver,
			int originator);

	int _port;
	int _maximum_connections;
	// Whether we are currently accepting connections
	bool _accepting_connections;
	PacketBuffer _packet_buffer;
	ENetHost* _server_socket;
	std::vector<ENetPeer*> _socket_list;
};

#endif
