/*
 * PacketQueue.h:
 *  A thread-safe packet queue.
 */

#ifndef PACKETQUEUE_H_
#define PACKETQUEUE_H_

#include <deque>
#include <SDL_mutex.h>
#include <enet/enet.h>

class PacketQueue {
public:
	PacketQueue();
	~PacketQueue();

	ENetPacket* wait_for_packet(int timeout);
	void queue_packet(ENetPacket* packet);
private:
	SDL_mutex* _queue_lock;
	SDL_cond* _has_packets;
	std::deque<ENetPacket*> _packets;
};

#endif /* PACKETQUEUE_H_ */
