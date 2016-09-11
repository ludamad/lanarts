/*
 * PacketQueue.cpp:
 *  A thread-safe packet queue.
 */

#include <mutex>
#include <chrono>

#include "PacketQueue.h"

ENetPacket* PacketQueue::wait_for_packet(int timeout) {
	ENetPacket* packet = NULL;
        std::unique_lock<std::mutex> lock(_queue_lock);


	if (_packets.empty()) {
		if (timeout <= 0) {
                    _has_packets.wait(lock);
		} else {
                    auto now = std::chrono::system_clock::now();
                    _has_packets.wait_until(lock, now + std::chrono::milliseconds(timeout));
		}
	}

	if (!_packets.empty()) {
		packet = _packets.front();
		_packets.pop_front();
	}

	return packet;
}

void PacketQueue::queue_packet(ENetPacket* packet) {
        std::unique_lock<std::mutex> lock(_queue_lock);
        _has_packets.notify_all();
	_packets.push_back(packet);
}
