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
static void complaining_close(UDTSOCKET socket) {
	if (UDT::close(socket) == UDT::ERROR) {
		fprintf(stderr, "Closing socket %d got error %s\n", socket,
				UDT::getlasterror().getErrorMessage());
	}
}

ServerConnection::~ServerConnection() {
	for (int i = 0; i < _socket_list.size(); i++) {
		complaining_close(_socket_list[i]);
		UDT::epoll_remove_usock(_poller, _socket_list[i]);
	}
	complaining_close(_server_socket);
	UDT::epoll_release(_poller);
}

void ServerConnection::initialize_connection() {
	if (!udt_initialize_connection(_server_socket, _port)) {
		return; //TODO return boolean
	}

	UDT::epoll_add_usock(_poller, _server_socket);
}

bool ServerConnection::_accept_connection() {
	sockaddr_storage clientaddr;
	int addrlen = sizeof(clientaddr);

	/* Make sure we accept in non-blocking mode */
	bool block = false;
	UDT::setsockopt(_server_socket, 0, UDT_RCVSYN, &block, sizeof(bool));
	UDTSOCKET client_socket = UDT::accept(_server_socket,
			(sockaddr*) &clientaddr, &addrlen);
	block = true;
	UDT::setsockopt(_server_socket, 0, UDT_RCVSYN, &block, sizeof(bool));

	if (client_socket == UDT::INVALID_SOCK) {
		if (UDT::getlasterror().getErrorCode() != CUDTException::EASYNCRCV) {
			__lnet_throw_connection_error("Accepting socket got error %s\n",
					UDT::getlasterror().getErrorMessage());
		}
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
	printf("Accepted a connection\n");
	return true;
}

bool ServerConnection::poll(packet_recv_callback message_handler, void* context,
		int timeout) {
	if (_server_socket == -1) {
		__lnet_throw_connection_error(
				"ServerConnection::poll: Connection not initialized!\n");
		return false;
	}

	while (true) {

		std::set<UDTSOCKET> sockets_to_read;
		int nready = UDT::epoll_wait(_poller, &sockets_to_read, NULL, timeout);
		timeout = 0; // Don't wait again on repeated checks

		if (nready == UDT::ERROR) {
			if (UDT::getlasterror().getErrorCode() != CUDTException::ETIMEOUT) {
				__lnet_throw_connection_error(
						"Error: UDT::epoll_wait reported error %s %d\n",
						UDT::getlasterror().getErrorMessage());
			}
			break;
		} else if (nready == 0) {
			break;
		}

		for (std::set<UDTSOCKET>::iterator sock_iter = sockets_to_read.begin();
				sock_iter != sockets_to_read.end(); ++sock_iter) {

			if (*sock_iter == _server_socket) {
				continue;
			}

			std::vector<UDTSOCKET>::iterator it = std::find(
					_socket_list.begin(), _socket_list.end(), *sock_iter);
			LNET_ASSERT(it != _socket_list.end());

			size_t i = it - _socket_list.begin();

			receiver_t receiver, _unused_sender;
			if (!receive_packet(*sock_iter, _packet_buffer, receiver,
					_unused_sender)) {
				return false;
			}

			if (!_packet_buffer.empty()) {
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
		_accept_connection();
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
		__lnet_throw_connection_error(
				"ServerConnection::send_message: Connection not initialized!\n");
	}

	prepare_packet(_packet_buffer, msg, len, receiver);
	_send_message(_packet_buffer, receiver, SERVER_RECEIVER);
}
