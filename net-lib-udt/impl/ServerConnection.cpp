#ifndef WIN32
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wspiapi.h>
#endif

#include <algorithm>

#include <udt/udt.h>

#include "udt-util.h"

#include "ServerConnection.h"

ServerConnection::ServerConnection(int port, int maximum_sockets) :
				_port(port),
				_maximum_sockets(maximum_sockets),
				_accepting_connections(true) {
	_server_socket = -1;
	_poller = UDT::epoll_create();
}

ServerConnection::~ServerConnection() {
	UDT::close(_server_socket);
}

void ServerConnection::initialize_connection() {
	if (!udt_initialize_connection(_server_socket, _port)) {
		return; //TODO return boolean
	}
}

bool ServerConnection::_accept_connection() {
	//			printf("ServerConnect::poll: new client connection\n");
	sockaddr_storage clientaddr;
	int addrlen = sizeof(clientaddr);
	bool block = false;
	UDT::setsockopt(_server_socket, 0, UDT_SNDSYN, &block, sizeof(bool));
	UDTSOCKET client_socket = UDT::accept(_server_socket,
			(sockaddr*) &clientaddr, &addrlen);
	block = true;
	UDT::setsockopt(_server_socket, 0, UDT_SNDSYN, &block, sizeof(bool));

	if (client_socket == UDT::INVALID_SOCK) {
		// No sockets to connect to
		return false;
	}

	if (!client_socket || !_accepting_connections
			|| _socket_list.size() >= _maximum_sockets) {
		printf("Rejecting attempted connection\n");
		UDT::close(client_socket);
	} else {
		UDT::epoll_add_usock(_poller, client_socket);
		_socket_list.push_back(client_socket);
	}
	return true;
}

bool ServerConnection::poll(packet_recv_callback message_handler, void* context,
		int timeout) {
	if (_server_socket == -1) {
		fprintf(stderr,
				"ServerConnection::poll: Connection not initialized!\n");
		return false;
	}

	while (true) {

		std::set<UDTSOCKET> sockets_to_read;
		int nready = UDT::epoll_wait(_poller, &sockets_to_read, NULL, timeout);
		timeout = 0; // Don't wait again on repeated checks

		if (nready < 0) {
			fprintf(stderr, "Error: UDT::epoll_wait reported error code %d\n",
					-nready);
			return false;
		} else if (nready == 0) {
			break;
		}

		for (std::set<UDTSOCKET>::iterator sock_iter = sockets_to_read.begin();
				sock_iter != sockets_to_read.end(); ++sock_iter) {
			std::vector<UDTSOCKET>::iterator it = std::find(
					_socket_list.begin(), _socket_list.end(), *sock_iter);
			LNET_ASSERT(it == _socket_list.end());

			size_t i = it - _socket_list.begin();

			receiver_t receiver, _unused_sender;
			if (!receive_packet(*sock_iter, _packet_buffer, receiver,
					_unused_sender)) {
				return false;
			}

			if (!_packet_buffer.empty()) {
				// Rebroadcast to clients
				// <TEMP>
				printf("Rebroadcasting message to %d from %d\n", receiver,
						i + 1);
				// </TEMP>
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

		while (_accept_connection()) {
			/* Accept connections until there are none left to accept */
		}
	}

	return true;
}

void ServerConnection::set_accepting_connections(bool accept) {
	_accepting_connections = accept;
}

void ServerConnection::_send_message(PacketBuffer& packet, receiver_t receiver,
		int originator) {
	set_packet_sender(packet, originator);
	if (receiver == ALL_RECEIVERS) {
		// <TEMP>
		std::string msg(&packet.at(12), packet.size() - 12);
		printf("ServerConnection::_send_message: Sending msg '%s'\n",
				msg.c_str());

		// </TEMP>
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
	if (_server_socket == -1) {
		fprintf(
				stderr,
				"ServerConnection::send_message: Connection not initialized!\n");
		fflush(stderr);
		return;
	}

	prepare_packet(_packet_buffer, msg, len, receiver);
	_send_message(_packet_buffer, receiver, SERVER_RECEIVER);
}
