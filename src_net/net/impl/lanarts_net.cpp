/*
 * lanarts_net.h:
 *  Primary functions for lanarts_net library
 */

#include "../lanarts_net.h"

#include "ClientConnection.h"
#include "ServerConnection.h"

/**
 * Initialize the library
 * return true on success
 */
bool lanarts_net_init(bool print_error) {
	if (SDLNet_Init() < 0) {
		if (print_error) {
			fprintf(stderr, "Couldn't initialize SDL_net: %s\n",
					SDL_GetError());
		}
		return false;
	}
	return true;
}

/**
 * Create a server at 'port'.
 * Note that you must first call initialize_connection().
 */
NetConnection* create_server_connection(int port) {
	return new ServerConnection(port);
}

/**
 * Create a client that will connect to a server with 'host' via 'port'.
 * Note that you must first call initialize_connection().
 */
NetConnection* create_client_connection(const char* host, int port) {
	return new ClientConnection(host, port);
}

