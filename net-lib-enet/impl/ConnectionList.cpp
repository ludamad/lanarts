/*
 * ConnectionList.cpp:
 *  A thread-safe list of peer connections.
 */

#include "ConnectionList.h"

ConnectionList::ConnectionList() {
	_list_lock = SDL_CreateMutex();
}

ConnectionList::~ConnectionList() {
	SDL_DestroyMutex(_list_lock);
}

int ConnectionList::add(ENetPeer* peer) {
	SDL_LockMutex(_list_lock);
	int idx = _peers.size();
	_peers.push_back(peer);
	SDL_UnlockMutex(_list_lock);
	return idx;
}

std::vector<ENetPeer*> ConnectionList::get() {
	SDL_LockMutex(_list_lock);
	std::vector<ENetPeer*> copy = _peers;
	SDL_UnlockMutex(_list_lock);
	return copy;
}

std::vector<ENetPeer*>& ConnectionList::unsafe_get() {
	return _peers;
}
//
//void ConnectionList::clear() {
//	SDL_LockMutex(_list_lock);
//	for (int i = 0; i < _peers.size(); i++) {
//		enet_peer_reset(_peers[i]);
//	}
//	_peers.clear();
//	SDL_UnlockMutex(_list_lock);
//}
