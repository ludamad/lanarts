/*
 * ServerConnection.h:
 *  Represents a connection made by a server to multiple clients
 */

#include "ServerConnection.h"

ServerConnection::ServerConnection(int port, int maximum_sockets) :
		_port(port), _maximum_sockets(maximum_sockets), _accepting_connections(
				true) {
	_socket_set = SDLNet_AllocSocketSet(maximum_sockets);
}

void ServerConnection::initialize_connection() {
	IPaddress ip;
	if (SDLNet_ResolveHost(&ip, NULL, _port) < 0) {
		fprintf(stderr, "Error resolving server host at port %d\n", _port);
	} else {
		_server_socket = SDLNet_TCP_Open(&ip);
		if (_server_socket) {
			SDLNet_TCP_AddSocket(_socket_set, _server_socket);
		} else {
			fprintf(stderr, "Error opening server socket\n");
		}
	}
}

ServerConnection::~ServerConnection() {
	SDLNet_TCP_Close(_server_socket);
	SDLNet_FreeSocketSet(_socket_set);
}

bool ServerConnection::poll(packet_recv_callback message_handler, void* context,
		int timeout) {
	if (_server_socket == NULL) {
		fprintf(stderr,
				"ServerConnection::poll: Connection not initialized!\n");
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

		for (int i = 0; i < _socket_list.size(); i++) {
			// If socket is ready, handle messages
			while (SDLNet_SocketReady(_socket_list[i])) {

//				printf("ServerConnect::poll: client socket %d activity\n",
//						i + 1);
				receiver_t receiver, _unused_sender;
				receive_packet(_socket_list[i], _packet_buffer, receiver,
						_unused_sender);
				// Rebroadcast to clients
				_send_message(_packet_buffer, receiver, receiver_t(i + 1));

				if (receiver == ALL_RECEIVERS || receiver == SERVER_RECEIVER) {
					if (message_handler) {
						message_handler(i + 1, context,
								&_packet_buffer[HEADER_SIZE],
								_packet_buffer.size() - HEADER_SIZE);
					}
				}
			}
		}

		while (SDLNet_SocketReady(_server_socket)) {
//			printf("ServerConnect::poll: new client connection\n");
			TCPsocket client_socket = SDLNet_TCP_Accept(_server_socket);
			if (!client_socket || !_accepting_connections
					|| _socket_list.size() >= _maximum_sockets) {
//				printf("ServerConnect::poll: client denied\n");
				SDLNet_TCP_Close(client_socket);
			} else {
				SDLNet_TCP_AddSocket(_socket_set, client_socket);
				_socket_list.push_back(client_socket);
			}
		}
	}

	return true;
}

void ServerConnection::set_accepting_connections(bool accept) {
	_accepting_connections = accept;
}

#include <string>

void ServerConnection::_send_message(PacketBuffer& packet, receiver_t receiver,
		int originator) {
	if (receiver == ALL_RECEIVERS) {
//		printf("Sending msg with packet buff size = %d\n", packet.size());
		std::string msg(&packet.at(12), packet.size() - 12);
//		printf("ServerConnection::_send_message: Sending msg '%s'\n",
//				msg.c_str());
		for (int i = 0; i < _socket_list.size(); i++) {
			if (i + 1 != originator) {
				send_packet(_socket_list[i], _packet_buffer);
			}
		}
	} else if (receiver > SERVER_RECEIVER) {
		send_packet(_socket_list[receiver - 1], _packet_buffer);
	}
}

void ServerConnection::send_message(const char* msg, int len,
		receiver_t receiver) {
	if (_server_socket == NULL) {
		fprintf(
				stderr,
				"ServerConnection::send_message: Connection not initialized!\n");
		return;
	}

	prepare_packet(_packet_buffer, msg, len, receiver);
	_send_message(_packet_buffer, receiver, SERVER_RECEIVER);
}

