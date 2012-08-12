/*
 * ClientConnection.cpp:
 *  Represents a client connected to a server
 */

#include "ClientConnection.h"

ClientConnection::ClientConnection(const char* addr, int port) :
		hostname(addr), _port(port) {
	_client_socket = NULL;
	_socket_set = SDLNet_AllocSocketSet(1);
}

ClientConnection::~ClientConnection() {
	SDLNet_TCP_Close(_client_socket);
	SDLNet_FreeSocketSet(_socket_set);
}

void ClientConnection::initialize_connection() {
	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, hostname.c_str(), _port) < 0) {
		fprintf(stderr, "Error resolving host name '%s'\n", hostname.c_str());
	} else {
		_client_socket = SDLNet_TCP_Open(&ip);
		if (!_client_socket) {
			fprintf(stderr,
					"Error resolving connecting to host '%s' with port %d\n",
					hostname.c_str(), _port);
		}

		SDLNet_TCP_AddSocket(_socket_set, _client_socket);
	}
}

bool ClientConnection::poll(packet_recv_callback message_handler, void* context, int timeout) {
	if (_client_socket == NULL) {
		fprintf(stderr,
				"ClientConnection::poll: Connection not initialized!\n");
		return false;
	}

	while (true) {
		int nready = SDLNet_CheckSockets(_socket_set, timeout);
		timeout = 0; // Don't wait again on repeated checks
		if (nready < 0) {
			fprintf(stderr, "Error: SDLNet_CheckSockets reported %s\n",
					SDLNet_GetError());
			return false;
		} else if (nready == 0) {
			break;
		}
		while (SDLNet_SocketReady(_client_socket)) {
			receiver_t receiver = receive_packet(_client_socket,
					_packet_buffer);
			if (message_handler) {
				message_handler(context, &_packet_buffer[HEADER_SIZE],
						_packet_buffer.size() - HEADER_SIZE);
			}
		}
	}
	return true;
}

void ClientConnection::send_message(const char* msg, int len,
		receiver_t receiver) {
	if (_client_socket == NULL) {
		fprintf(
				stderr,
				"ClientConnection::send_message: Connection not initialized!\n");
		return;
	}

	prepare_packet(_packet_buffer, msg, len, receiver);
	send_packet(_client_socket, _packet_buffer);
}
