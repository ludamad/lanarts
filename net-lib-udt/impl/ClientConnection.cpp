/*
 * ClientConnection.cpp:
 *  Represents a client connected to a server
 */

#include <udt/udt.h>

#include "udt-util.h"

#include "../lanarts_net.h"
#include "ClientConnection.h"

ClientConnection::ClientConnection(const char* addr, int port) :
				hostname(addr),
				_port(port) {
	_client_socket = -1;
	_poller = UDT::epoll_create();
}

ClientConnection::~ClientConnection() {
	UDT::close(_client_socket);
	UDT::epoll_release(_poller);
}

void ClientConnection::initialize_connection() {
	if (!udt_initialize_connection(_client_socket, _port, hostname.c_str())) {
		__lnet_throw_connection_error("Failed to initialize connection.");
	}

	UDT::epoll_add_usock(_poller, _client_socket);
}

bool ClientConnection::poll(packet_recv_callback message_handler, void* context,
		int timeout) {
	if (_client_socket == -1) {
		__lnet_throw_connection_error(
				"ClientConnection::poll: Connection not initialized!\n");
		return false;
	}

	while (true) {
		std::set<UDTSOCKET> sockets_to_read;
		int nready = UDT::epoll_wait(_poller, &sockets_to_read, NULL, timeout);
		timeout = 0; // Don't wait again on repeated checks

		if (nready == UDT::ERROR) {
			if (UDT::getlasterror().getErrorCode() != CUDTException::ETIMEOUT) {
				__lnet_throw_connection_error(
						"Error: UDT::epoll_wait reported error %s\n",
						UDT::getlasterror().getErrorMessage());
			} else if (!sockets_to_read.empty()) {
				__lnet_throw_connection_error("Socket closed or broken\n");
			}
			break;
		} else if (nready == 0) {
			break;
		}
		receiver_t receiver, sender;

		if (!receive_packet(_client_socket, _packet_buffer, receiver, sender)) {
			return false;
		}

		if (message_handler && !_packet_buffer.empty()) {
			message_handler(sender, context, &_packet_buffer[HEADER_SIZE],
					_packet_buffer.size() - HEADER_SIZE);
		}
	}
	return true;
}

void ClientConnection::send_message(const char* msg, int len,
		receiver_t receiver) {
	if (_client_socket == -1) {
		__lnet_throw_connection_error(
				"ClientConnection::send_message: Connection not initialized!\n");
		return;
	}

	prepare_packet(_packet_buffer, msg, len, receiver);
	send_packet(_client_socket, _packet_buffer);
}
