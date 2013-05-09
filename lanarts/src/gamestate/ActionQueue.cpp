/*
 * ActionQueue.h:
 *  Provides ActionQueue class, which is an alias for a deque.
 *  As well provides MultiframeActionQueue, which queues actions for specific frames.
 */

#include "ActionQueue.h"

bool MultiframeActionQueue::extract_actions_for_frame(ActionQueue & actions,
		int frame) {

	if (!has_actions_for_frame(frame)) {
		return false;
	}

	actions = queue[frame];

	return true;
}

bool MultiframeActionQueue::has_actions_for_frame(int frame) {
	return queue.count(frame) != 0;
}

void MultiframeActionQueue::queue_actions_for_frame(const ActionQueue & actions,
		int frame) {
	LANARTS_ASSERT(!has_actions_for_frame(frame));
	queue[frame] = actions;
}

void MultiframeActionQueue::clear() {
	queue.clear();
}

void MultiframeActionQueue::clear_actions() {
	std::map<int, ActionQueue>::iterator it = queue.begin();
	for (; it != queue.end(); ++it) {
		it->second.clear();
	}
}
