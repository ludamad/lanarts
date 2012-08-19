/*
 * ActionQueue.h:
 *  Provides ActionQueue class, which is an alias for a deque.
 */

#ifndef ACTIONQUEUE_H_
#define ACTIONQUEUE_H_

#include <deque>

#include "GameAction.h"

typedef std::deque<GameAction> ActionQueue;

/* Buffer for actions sent for future frames */
class MultiframeActionQueue {
public:
	void extract_actions_for_frame(ActionQueue& actions, int frame);
	bool has_actions_for_frame(int frame);
	void queue_actions_for_frame(const ActionQueue& actions, int frame);
	void clear_actions() {
		queue.clear();
	}
	GameAction& at(int i) {
		return queue.at(i);
	}
private:
	ActionQueue queue;
	std::deque<int> frames_set;
};

#endif /* ACTIONQUEUE_H_ */
