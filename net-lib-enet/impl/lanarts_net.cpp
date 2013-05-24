/*
 * lanarts_net.h:
 *  Primary functions for lanarts_net library
 */

#include <enet/enet.h>

#include <lcommon/strformat.h>

#include "../lanarts_net.h"

#include "ClientConnection.h"
#include "ServerConnection.h"

/**
 * Initialize the library
 * return true on success
 */
bool lanarts_net_init(bool throw_on_error) {
	if (enet_initialize() != 0) {
		if (throw_on_error) {
			__lnet_throw_connection_error("Couldn't initialize SDL_net: %s\n");
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

void lanarts_net_quit() {
	enet_deinitialize();
}

/**
 * Create a client that will connect to a server with 'host' via 'port'.
 * Note that you must first call initialize_connection().
 */
NetConnection* create_client_connection(const char* host, int port) {
	return new ClientConnection(host, port);
}


void __lnet_throw_connection_error(const char* fmt, ...) {
	std::string err;
	VARARG_STR_FORMAT(err, fmt);
#ifndef LNET_NO_EXCEPTIONS
	throw LNetConnectionError(err);
#else
	fputs(stderr, err.c_str());
	abort();
#endif
}
