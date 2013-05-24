/*
 * ServerConnection.h:
 *  Represents a connection made by a server to multiple clients
 */

#include "../lanarts_net.h"
#include "enet_util.h"

#include "ServerConnection.h"

ServerConnection::ServerConnection(int port, int maximum_connections) {
	_port = port;
	_maximum_connections = maximum_connections;
	_accepting_connections = true;
	_server_socket = NULL;
}

ServerConnection::~ServerConnection() {
	for (int i = 0; i < _socket_list.size(); i++) {
		enet_peer_reset(_socket_list[i]);
	}
	enet_host_destroy(_server_socket);
}

void ServerConnection::initialize_connection() {
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = _port;
	_server_socket = enet_host_create(
			&address /* the address to bind the server host to */,
			32 /* allow up to 32 clients and/or outgoing connections */,
			1 /* one channel, channel 0 */,
			0 /* assume any amount of incoming bandwidth */,
			0 /* assume any amount of outgoing bandwidth */);

	if (_server_socket == NULL) {
		__lnet_throw_connection_error(
				"An error occurred while trying to create an ENet server host.\n");
	}
	printf("ServerSocket connected\n");
}

bool ServerConnection::poll(packet_recv_callback message_handler, void* context,
		int timeout) {
	ENetEvent event;

	while (poll_adapter(_server_socket, &event, timeout) > 0) {
		timeout = 0; // Don't wait for timeout a second time
		switch (event.type) {
		case ENET_EVENT_TYPE_CONNECT:
			char clientname[1024];
			enet_address_get_host(&event.peer->address, clientname, 1024);
			printf("A new client connected from %s:%u.\n", clientname,
					event.peer->address.port);
			_socket_list.push_back(event.peer);

			/* Store client ID. */
			event.peer->data = (void*) _socket_list.size();
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			printf(
					"A packet of length %u containing %s was received from sender=%d on channel %u.\n",
					event.packet->dataLength, event.packet->data + HEADER_SIZE,
					event.peer->data, event.channelID);
			/* Clean up the packet now that we're done using it. */
			receiver_t receiver, _unused_sender;
			copy_and_release_packet(event.packet, _packet_buffer, receiver, _unused_sender);

			if (!_packet_buffer.empty()) {
				// Rebroadcast to clients
				int sender_id = (int)(long)event.peer->data;
				printf("Rebroadcasting message to %d from %d\n", receiver, sender_id);
				_send_message(_packet_buffer, receiver, (receiver_t)sender_id);

				if (receiver == ALL_RECEIVERS
						|| receiver == SERVER_RECEIVER) {
					if (message_handler) {
						message_handler(sender_id, context,
								&_packet_buffer[HEADER_SIZE],
								_packet_buffer.size() - HEADER_SIZE);
					}
				}
			}
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			printf("client=%d disconnected.\n", event.peer->data);
			break;
		}
	}
	return true;
}

void ServerConnection::set_accepting_connections(bool accept) {
	_accepting_connections = accept;
}

void ServerConnection::send_message(const char* msg, int len,
		receiver_t receiver) {
	if (_server_socket == NULL) {
		__lnet_throw_connection_error(
				"ServerConnection::send_message: Connection not initialized!\n");
	}

	prepare_packet(_packet_buffer, msg, len, receiver);
	_send_message(_packet_buffer, receiver, SERVER_RECEIVER);
}

void ServerConnection::_send_message(PacketBuffer& packet, receiver_t receiver,
		int originator) {
	set_packet_sender(packet, originator);

	printf("Sending msg with packet buff size = %d\n", packet.size());
	std::string msg(&packet.at(HEADER_SIZE), packet.size() - HEADER_SIZE);
	printf("ServerConnection::_send_message: Sending msg '%s'\n",
			msg.c_str());
	if (receiver == ALL_RECEIVERS) {
		if (originator == 0) {
			broadcast_packet(_server_socket, _packet_buffer);
		} else {
			for (int i = 0; i < _socket_list.size(); i++) {
				if (i + 1 != originator) {
					send_packet(_socket_list[i], _packet_buffer);
				}
			}
		}
	} else if (receiver > SERVER_RECEIVER) {
		send_packet(_socket_list[receiver - 1], _packet_buffer);
	}

	enet_host_flush(_server_socket);
}
