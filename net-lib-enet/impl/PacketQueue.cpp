/*
 * PacketQueue.cpp:
 *  A thread-safe packet queue.
 */

#include "PacketQueue.h"

PacketQueue::PacketQueue() {
	_queue_lock = SDL_CreateMutex();
	_has_packets = SDL_CreateCond();
}

PacketQueue::~PacketQueue() {
	SDL_DestroyMutex(_queue_lock);
	SDL_DestroyCond(_has_packets);
}

ENetPacket* PacketQueue::wait_for_packet(int timeout) {
	ENetPacket* packet = NULL;
	SDL_LockMutex(_queue_lock);

	if (_packets.empty()) {
		if (timeout < 0) {
			SDL_CondWait(_has_packets, _queue_lock);
		} else {
			SDL_CondWaitTimeout(_has_packets, _queue_lock, timeout);
		}
	}

	if (!_packets.empty()) {
		packet = _packets.front();
		_packets.pop_front();
	}

	SDL_UnlockMutex(_queue_lock);
	return packet;
}

void PacketQueue::queue_packet(ENetPacket* packet) {
	SDL_LockMutex(_queue_lock);
	SDL_CondSignal(_has_packets);
	_packets.push_back(packet);
	SDL_UnlockMutex(_queue_lock);
}
