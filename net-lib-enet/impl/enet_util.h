/*
 * enet_util.h:
 *  Common utility functions for the enet-implemented server & client.
 */

#ifndef ENET_UTIL_H_
#define ENET_UTIL_H_

#include <cstdio>
#include <enet/enet.h>

/*
 * Wait forever if we get a negative number
 */
inline int poll_adapter(ENetHost* host, ENetEvent* event, int timeout) {
	bool forever = false;
	if (timeout < 0) {
		forever = true;
		timeout = 1000000; // Arbitrary large number
	}
	printf("Polling with timeout %d\n", timeout);
	int result;
	do {
		result = enet_host_service(host, event, timeout);
		if (result > 0) {
			return result;
		}
	} while (forever);
	return result;
}

#endif /* ENET_UTIL_H_ */
