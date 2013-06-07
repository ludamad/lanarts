/*
 * ServerConnection.h:
 *  Represents a connection made by a server to multiple clients
 */

#ifndef SERVERCONNECTION_H_
#define SERVERCONNECTION_H_

#include <vector>

#include <SDL/SDL.h>
#include <SDL/SDL_mutex.h>
#include <enet/enet.h>

#include "../NetConnection.h"

#include "ConnectionList.h"
#include "PacketQueue.h"
#include "packet_util.h"

/* Shared with the polling thread & the main thread */
struct ServerConnectionData {
	ServerConnectionData();
	~ServerConnectionData();
	/* Can be set by other thread in case we are destroyed */
	volatile bool destroyed;
	volatile bool disconnect;

	/* Used when sending and receiving messages on the polling thread */
	PacketBuffer polling_buffer;
	/* The server socket that listens for connections */
	ENetHost* server_socket;
	/* Packets queued up by the polling thread*/
	PacketQueue packet_queue;
	/* List of currently connected clients */
	ConnectionList connections;
	/* Lock to ensure the main & polling thread don't send packets at once
	 * This is needed because enet's sending mechanism is not thread-safe */
	SDL_mutex* packet_send_mutex;
};

class ServerConnection: public NetConnection {
public:
	ServerConnection(int port, int maximum_connections = 32);
	virtual ~ServerConnection();

	virtual void initialize_connection();

	virtual int poll(packet_recv_callback message_handler,
			void* context = NULL, int timeout = 0);
	virtual void set_accepting_connections(bool accept);
	virtual void send_message(const char* msg, int len, receiver_t receiver =
			ALL_RECEIVERS);

	std::vector<ENetPeer*> get_socket_list() {
		return _data->connections.get();
	}
private:
	static int server_poll_thread(void* context);

	SDL_Thread* _polling_thread;
	ServerConnectionData* _data;
	int _port;
	int _maximum_connections;
	// Whether we are currently accepting connections
	bool _accepting_connections;
	/* Used when sending messages on the main thread */
	PacketBuffer _packet_buffer;
};

#endif
