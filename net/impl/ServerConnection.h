/*
 * ServerConnection.h:
 *  Represents a connection made by a server to multiple clients
 */

#ifndef SERVERCONNECTION_H_
#define SERVERCONNECTION_H_

#include <vector>

#include <SDL_net.h>

#include "../NetConnection.h"

#include "packet_util.h"

class ServerConnection: public NetConnection {
public:
	ServerConnection(int port, int maximum_sockets = 32);
	virtual ~ServerConnection();

	virtual void initialize_connection();

	virtual bool poll(packet_recv_callback message_handler,
			void* context = NULL, int timeout = 0);
	virtual void set_accepting_connections(bool accept);
	virtual void send_message(const char* msg, int len, receiver_t receiver =
			ALL_RECEIVERS);

private:
	void _send_message(PacketBuffer& packet, receiver_t receiver,
			int originator);
	int _port;
	// Buffer for sending & receiving messages
	PacketBuffer _packet_buffer;
	// Maximum accepted connections
	int _maximum_sockets;
	// Whether we are currently accepting connections
	bool _accepting_connections;
	TCPsocket _server_socket;
	// List of client sockets
	std::vector<TCPsocket> _socket_list;
	SDLNet_SocketSet _socket_set;
};

#endif /* SERVERCONNECTION_H_ */
