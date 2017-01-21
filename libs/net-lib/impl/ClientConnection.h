/*
 * ClientConnection.h:
 *  Represents a client connected to a server
 */

#ifndef CLIENTCONNECTION_H_
#define CLIENTCONNECTION_H_

#include <string>
#include <vector>
#include <functional>

#include "../NetConnection.h"

#include "packet_util.h"

class ClientConnection: public NetConnection {
public:
	ClientConnection(const char* addr, int port);
	virtual ~ClientConnection();

	virtual void initialize_connection(const conn_callback &callback, int timeout);

	virtual int poll(packet_recv_callback message_handler, void* context = NULL, int timeout = 0);
	virtual void set_accepting_connections(bool accept) {
		//no-op
	}
	virtual void send_message(const char* msg, int len, receiver_t receiver =
			ALL_RECEIVERS);
private:
	std::string _hostname;
	int _port;
	// Buffer for sending & receiving messages
	PacketBuffer _packet_buffer;
	ENetHost* _client_socket;
	ENetPeer* _server_peer;
};

#endif /* CLIENTCONNECTION_H_ */
