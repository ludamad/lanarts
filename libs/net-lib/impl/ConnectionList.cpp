/*
 * ConnectionList.cpp:
 *  A thread-safe list of peer connections.
 */

#include "ConnectionList.h"

ConnectionList::ConnectionList() {
}

ConnectionList::~ConnectionList() {
}

int ConnectionList::add(ENetPeer* peer) {
        std::unique_lock<std::mutex> lock(_list_lock);
	int idx = _peers.size();
	_peers.push_back(peer);
	return idx;
}

std::vector<ENetPeer*> ConnectionList::get() {
        std::unique_lock<std::mutex> lock(_list_lock);
	std::vector<ENetPeer*> copy = _peers;
	return copy;
}

std::vector<ENetPeer*>& ConnectionList::unsafe_get() {
	return _peers;
}
//
//void ConnectionList::clear() {
//      std::unique_lock<std::mutex> lock(_list_lock);
//	for (int i = 0; i < _peers.size(); i++) {
//		enet_peer_reset(_peers[i]);
//	}
//	_peers.clear();
//}
