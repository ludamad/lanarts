/*
 * ClientConnection.h:
 *  Represents a client connected to a server
 */

#include <functional>
#include "ClientConnection.h"

#include "enet_util.h"
#include "../lanarts_net.h"

ClientConnection::ClientConnection(const char* addr, int port) {
	_hostname = addr;
	_port = port;
	_client_socket = NULL;
	_server_peer = NULL;
}

ClientConnection::~ClientConnection() {
	enet_peer_disconnect(_server_peer, 0);
//	enet_peer_reset(_server_peer);
	enet_host_destroy(_client_socket);
}

/*!
 * Try to connect to a given host with a given timeout
 * @param host
 * @param callback should return a bool indicating desire to continue trying
 * @param timeout in milliseconds
 * @return true if connection, false otherwise
 */
static bool wait_for_connect(ENetHost* host, std::function<bool()> callback, int timeout) {
	ENetEvent event;
	while(callback()) {
		if (enet_host_service(host, &event, timeout) > 0) {
			switch (event.type) {
				case ENET_EVENT_TYPE_RECEIVE:
					printf("RECV\n"); break;
				case ENET_EVENT_TYPE_DISCONNECT:
					printf("DISC\n"); break;
				case ENET_EVENT_TYPE_CONNECT:
					printf("CONN\n");
					return true;
			}
		}
	}
	return false;
}

/*!
 * Try to initialize connection
 * @param callback on every connection timeout
 * @param timeout in ms
 */
void ClientConnection::initialize_connection(std::function<bool()> callback, int timeout) {
	_client_socket = enet_host_create(NULL,
			32 /* allow up to 32 clients and/or outgoing connections */,
			1 /* one channel, channel 0 */,
			0 /* assume any amount of incoming bandwidth */,
			0 /* assume any amount of outgoing bandwidth */);

	if (_client_socket == NULL) {
		__lnet_throw_connection_error(
				"An error occurred while trying to create an ENet client host at port %d.\n", _port);
	}

	ENetAddress address;
	enet_address_set_host(&address, _hostname.c_str());
	address.port = _port;

	_server_peer = enet_host_connect(_client_socket, &address, 2, 0);
	if (_server_peer == NULL) {
		__lnet_throw_connection_error(
				"No available peers for initiating an ENet connection.\n");
	}
	printf("ClientSocket waiting for %s:%d\n", _hostname.c_str(), _port);

    if ( !wait_for_connect(_client_socket, callback, timeout)) {
        __lnet_throw_connection_error("Connection attempt was cancelled.");
    }
}

int ClientConnection::poll(packet_recv_callback message_handler, void* context,
		int timeout) {
	ENetEvent event;

	int polled = 0;
	while (poll_adapter(_client_socket, &event, timeout) > 0) {
		timeout = 0; // Don't wait for timeout a second time -- consume remaining events in buffer
		switch (event.type) {
		case ENET_EVENT_TYPE_RECEIVE: {
			ENetPacket* epacket = event.packet;
			int sender_id = get_epacket_sender(epacket);
			if (message_handler) {
				polled++;
				message_handler(sender_id, context,
						HEADER_SIZE + (const char*)epacket->data,
						epacket->dataLength - HEADER_SIZE);
			}
			enet_packet_destroy(epacket);
			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
			printf("We have disconnected from the server.\n");
			__lnet_throw_connection_error("Client connection dropped");
			break;
		}
	}
	return polled;
}

void ClientConnection::send_message(const char* msg, int len,
		receiver_t receiver) {
	if (_client_socket == NULL) {
		__lnet_throw_connection_error(
				"ServerConnection::send_message: Connection not initialized!\n");
	}

	prepare_packet(_packet_buffer, msg, len, receiver);
	send_packet(_server_peer, _packet_buffer);
	enet_host_flush(_client_socket);
}
