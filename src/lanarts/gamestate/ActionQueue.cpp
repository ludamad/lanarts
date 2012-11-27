/*
 * ActionQueue.h:
 *  Provides ActionQueue class, which is an alias for a deque.
 *  As well provides MultiframeActionQueue, which queues actions for specific frames.
 */

#include "ActionQueue.h"

void MultiframeActionQueue::extract_actions_for_frame(ActionQueue & actions,
		int frame) {
	actions.clear();
	while (!queue.empty() && queue.front().frame <= frame) {
		actions.push_back(queue.front());
		queue.pop_front();
	}
}

bool MultiframeActionQueue::has_actions_for_frame(int frame) {
	for (int i = 0; i < frames_set.size() && frames_set[i] <= frame; i++) {
		if (frames_set[i] == frame) {
			return true;
		}
	}
	return false;
}

void MultiframeActionQueue::queue_actions_for_frame(const ActionQueue & actions,
		int frame) {
	frames_set.push_back(frame);
	for (int i = 0; i < actions.size(); i++) {
		queue.push_back(actions[i]);
	}
}

