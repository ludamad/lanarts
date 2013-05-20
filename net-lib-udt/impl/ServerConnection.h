#ifndef SERVERCONNECTION_H_
#define SERVERCONNECTION_H_

#include <udt/udt.h>
#include <udt/epoll.h>

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
	bool _accept_connection();
	void _send_message(PacketBuffer& packet, receiver_t receiver,
			int originator);

	int _port;
	int _maximum_sockets;
	// Whether we are currently accepting connections
	bool _accepting_connections;
	PacketBuffer _packet_buffer;
	UDTSOCKET _server_socket;
	std::vector<UDTSOCKET> _socket_list;

	SYSSOCKET _poller;
};

#endif
