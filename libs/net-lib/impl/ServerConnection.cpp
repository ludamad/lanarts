/*
 * ServerConnection.h:
 *  Represents a connection made by a server to multiple clients
 */

#include <vector>

#include <SDL_thread.h>
#include <SDL_mutex.h>

#include <lcommon/Timer.h>
#include <functional>

#include "../lanarts_net.h"

#include "enet_util.h"
#include "PacketQueue.h"
#include "ServerConnection.h"

const int SERVER_POLL_TIME = 200; /* 200 milliseconds */

static void server_send_packet(SDL_mutex* mutex, ENetHost* server,
		const std::vector<ENetPeer*>& peers, PacketBuffer& packet,
		receiver_t receiver, int originator) {
	SDL_LockMutex(mutex);
	std::string msg(&packet.at(HEADER_SIZE), packet.size() - HEADER_SIZE);
	if (receiver == NetConnection::ALL_RECEIVERS) {
		if (originator == 0) {
			broadcast_packet(server, packet);
		} else {
			for (int i = 0; i < peers.size(); i++) {
				if (i + 1 != originator) {
					send_packet(peers[i], packet);
				}
			}
		}
		enet_host_flush(server);
	} else if (receiver > NetConnection::SERVER_RECEIVER) {
		send_packet(peers[receiver - 1], packet);
		enet_host_flush(server);
	}
	SDL_UnlockMutex(mutex);
}

ServerConnectionData::ServerConnectionData() :
				destroyed(false),
				server_socket(NULL),
				disconnect(false) {
	packet_send_mutex = SDL_CreateMutex();
}

ServerConnectionData::~ServerConnectionData() {
	enet_host_destroy(server_socket);
	SDL_DestroyMutex(packet_send_mutex);
}

// Wraps enet_host_service in a thread-safe manner
// We must use the packet sending mutex because enet_host_service will try to send packets
static int thread_safe_host_service(ServerConnectionData* server_data, ENetHost* host,
		ENetEvent* event, int timeout) {
	Timer timer;
	while (!server_data->destroyed) {
		SDL_LockMutex(server_data->packet_send_mutex);
		int event_status = enet_host_service(host, event, 0);
		SDL_UnlockMutex(server_data->packet_send_mutex);
		if (event_status < 0) {
			server_data->disconnect = true;
		}
		if (event_status > 0) {
			return event_status;
		}
		int time_left = timeout - timer.get_microseconds() / 1000;
		if (time_left <= 0) {
			break;
		}
		unsigned int condition = ENET_SOCKET_WAIT_RECEIVE;
		if (enet_socket_wait(host->socket, &condition, time_left) != 0) {
			__lnet_throw_connection_error(
					"Error when waiting on socket in 'thread_safe_host_service'\n");
		}
	}
	return 0;
}

int ServerConnection::server_poll_thread(void* context) {
	ServerConnectionData* server_data = (ServerConnectionData*) context;
	ENetHost* host = server_data->server_socket;
	PacketQueue& packet_queue = server_data->packet_queue;
	ConnectionList& connections = server_data->connections;
	PacketBuffer& packet_buffer = server_data->polling_buffer;

	while (!server_data->destroyed) {
		ENetEvent event;
		if (thread_safe_host_service(server_data, host, &event,
				SERVER_POLL_TIME) == 0) {
			continue;
		}
		do {
			/* Loop over all immediately available actions */
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT: {
				char clientname[1024];
				enet_address_get_host(&event.peer->address, clientname, 1024);
				printf("A new client connected from %s:%u.\n", clientname,
						event.peer->address.port);
				int idx = connections.add(event.peer);

				/* Store client ID. */
				event.peer->data = (void*) (long) (idx + 1);
				break;
			}

			case ENET_EVENT_TYPE_RECEIVE: {
				receiver_t receiver, _unused_sender;
				ENetPacket* epacket = event.packet;
				int sender_id = (int) (long) event.peer->data;
				set_epacket_sender(epacket, sender_id);
				copy_packet_to_buffer(epacket, packet_buffer, receiver,
						_unused_sender);

				if (receiver == NetConnection::ALL_RECEIVERS
						|| receiver == NetConnection::SERVER_RECEIVER) {
					packet_queue.queue_packet(epacket);
				}

				// Rebroadcast to clients
				/* NB: We are the only writer to 'connections', we can use an unsafe grab */
				server_send_packet(server_data->packet_send_mutex, host,
						connections.unsafe_get(), packet_buffer, receiver,
						(receiver_t) sender_id);
				break;
			}

			case ENET_EVENT_TYPE_DISCONNECT: {
				printf("client=%d disconnected.\n", event.peer->data);
				server_data->disconnect = true;
				break;
			}
			}
		} while (thread_safe_host_service(server_data, host, &event, 0));
	}

	delete server_data;

	return 0;
}

ServerConnection::ServerConnection(int port, int maximum_connections) {
	_port = port;
	_maximum_connections = maximum_connections;
	_accepting_connections = true;
	_data = NULL;
	_polling_thread = NULL;
}

ServerConnection::~ServerConnection() {
	/* Let the polling thread clean up!*/
	if (_data != NULL) {
		_data->destroyed = true;
		/* Disconnecting clients hopefully will wake up server thread */
		if (_polling_thread != NULL) {
			SDL_WaitThread(_polling_thread, NULL);
		}
	}
}

void ServerConnection::initialize_connection(const conn_callback &callback, int timeout) {
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = _port;

	_data = new ServerConnectionData();
	_data->server_socket = enet_host_create(&address, _maximum_connections,
			1 /* one channel, channel 0 */,
			0 /* assume any amount of incoming bandwidth */,
			0 /* assume any amount of outgoing bandwidth */);

	if (_data->server_socket == NULL) {
		__lnet_throw_connection_error(
				"An error occurred while trying to create an ENet server host.\n");
	}
	_polling_thread = SDL_CreateThread(server_poll_thread, "server-polling-thread", _data);
	if (!_polling_thread) {
		__lnet_throw_connection_error(
				"An error occurred while trying to create the server connection polling thread.\n");
	}
}

int ServerConnection::poll(packet_recv_callback message_handler, void* context,
		int timeout) {
	int polled = 0;
	while (true) {
		if (_data->disconnect) {
			__lnet_throw_connection_error("Server disconnect!");
		}
		ENetPacket* epacket = _data->packet_queue.wait_for_packet(timeout);
		if (epacket == NULL) {
			break; /* Nothing to read */
		}
		timeout = 0; // Don't wait a second time
		if (message_handler) {
			polled++;
			message_handler(get_epacket_sender(epacket), context,
					(const char*) &epacket->data[HEADER_SIZE],
					epacket->dataLength - HEADER_SIZE);
		}
		enet_packet_destroy(epacket);
	}
	return polled;
}

void ServerConnection::set_accepting_connections(bool accept) {
	_accepting_connections = accept;
}

void ServerConnection::send_message(const char* msg, int len,
		receiver_t receiver) {
	if (_data->server_socket == NULL) {
		__lnet_throw_connection_error(
				"ServerConnection::send_message: Connection not initialized!\n");
	}

	prepare_packet(_packet_buffer, msg, len, receiver);
	server_send_packet(_data->packet_send_mutex, _data->server_socket,
			_data->connections.get(), _packet_buffer, receiver,
			SERVER_RECEIVER);
}
