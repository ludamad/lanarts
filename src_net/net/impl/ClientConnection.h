/*
 * ClientConnection.h:
 *  Represents a client connected to a server
 */

#ifndef CLIENTCONNECTION_H_
#define CLIENTCONNECTION_H_

#include <string>
#include <vector>

#include <SDL_net.h>

#include "../NetConnection.h"
#include "packet_util.h"

class ClientConnection: public NetConnection {
public:
	ClientConnection(const char* addr, int port);
	virtual ~ClientConnection();

	virtual void initialize_connection();

	virtual bool poll(packet_recv_callback message_handler, void* context = NULL, int timeout = 0);
	virtual void set_accepting_connections(bool accept) {
		//no-op
	}
	virtual void send_message(const char* msg, int len, receiver_t receiver =
			ALL_RECEIVERS);
private:
	std::string hostname;
	int _port;
	// Buffer for sending & receiving messages
	PacketBuffer _packet_buffer;
	TCPsocket _client_socket;
	SDLNet_SocketSet _socket_set;
};

#endif /* CLIENTCONNECTION_H_ */
