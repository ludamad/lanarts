/*
 * enet_util.h:
 *  Common utility functions for the enet-implemented server & client.
 */

#ifndef ENET_UTIL_H_
#define ENET_UTIL_H_

#include <cstdio>
#include <enet/enet.h>
#include "net-lib/lanarts_net.h"

/*
 * Lanarts peers can stall for a long time (eg while a level generates), so allow ten times
 * ENet's default timeouts before a peer is considered disconnected.
 */
inline void set_lanarts_peer_timeout(ENetPeer* peer) {
	enet_peer_timeout(peer, ENET_PEER_TIMEOUT_LIMIT, 50000, 300000);
}

/*
 * Wait forever if we get a negative number
 */
inline int poll_adapter(ENetHost* host, ENetEvent* event, int timeout) {
	bool forever = false;
	if (timeout < 0) {
		forever = true;
		timeout = 1000000; // Arbitrary large number
	}
	int result;
	do {
		result = enet_host_service(host, event, timeout);
		if (result < -1) {
			__lnet_throw_connection_error("Client connection dropped");
		}
		if (result > 0) {
			return result;
		}
	} while (forever);
	return result;
}

#endif /* ENET_UTIL_H_ */
