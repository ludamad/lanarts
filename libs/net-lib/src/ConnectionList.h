/*
 * ConnectionList.h:
 *  A thread-safe list of peer connections.
 */

#ifndef CONNECTIONLIST_H_
#define CONNECTIONLIST_H_

#include <vector>
#include <SDL_mutex.h>
#include <enet/enet.h>

class ConnectionList {
public:
	ConnectionList();
	~ConnectionList();

	int add(ENetPeer* peer);
	std::vector<ENetPeer*> get();
	/* If there is only one writer thread, it is OK for it to read it without a lock*/
	std::vector<ENetPeer*>& unsafe_get();
private:
	SDL_mutex* _list_lock;
	std::vector<ENetPeer*> _peers;
};

#endif /* CONNECTIONLIST_H_ */
