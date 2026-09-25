/*
 * lanarts_net.h:
 *  Primary header for lanarts_net library
 */

#ifndef LANARTS_NET_H_
#define LANARTS_NET_H_

#include "NetConnection.h"

/**
 * Initialize the library
 * return true on success
 */
bool lanarts_net_init(bool throw_on_error = true);
// NB: does not abort
void lanarts_net_quit();

/**
 * Create a server at 'port'.
 * Note that you must call initialize_connection().
 */
NetConnection* create_server_connection(int port);

/**
 * Create a client that will connect to a server with 'host' via 'port'.
 * Note that you must call initialize_connection().
 */
NetConnection* create_client_connection(const char* host, int port);

#ifndef LNET_NO_EXCEPTIONS

#include <stdexcept>

class LNetConnectionError: public std::runtime_error {
public:
	LNetConnectionError(const std::string& err) :
					runtime_error(err) {
	}
};
#endif

void __lnet_throw_connection_error(const char* fmt, ...);

#endif /* LANARTS_NET_H_ */
