// Simple tests for utility classes

#include <SDL.h>
#include <lcommon/unittest.h>
#include "../impl/PacketQueue.h"

static int writer_thread(void* _queue) {
	PacketQueue& queue = *(PacketQueue*)_queue;
	for (long i = 0; i < 1000; i++) {
		queue.queue_packet((ENetPacket*)i);
	}

	return 0;
}

TEST(packet_queue) {
	PacketQueue queue;
	/* Ensure we get sequential packets */
	SDL_CreateThread(writer_thread, "packet-queue-thread", &queue);
	for (int i = 0; i < 1000; i++) {
		ENetPacket* pseudo_packet = queue.wait_for_packet(-1);
		CHECK_EQUAL(i, (long)pseudo_packet);
	}
}
