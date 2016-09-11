/*
 * PacketQueue.h:
 *  A thread-safe packet queue.
 */

#ifndef PACKETQUEUE_H_
#define PACKETQUEUE_H_

#include <mutex>
#include <condition_variable>
#include <deque>
#include <enet/enet.h>

class PacketQueue {
public:
	ENetPacket* wait_for_packet(int timeout);
	void queue_packet(ENetPacket* packet);
private:
        std::mutex _queue_lock;
        std::condition_variable _has_packets;
	std::deque<ENetPacket*> _packets;
};

#endif /* PACKETQUEUE_H_ */
