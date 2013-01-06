/*
 * ActionQueue.h:
 *  Provides ActionQueue class, which is an alias for a deque.
 *  As well provides MultiframeActionQueue, which queues actions for specific frames.
 */

#ifndef ACTIONQUEUE_H_
#define ACTIONQUEUE_H_

#include <deque>
#include <map>

#include "GameAction.h"

typedef std::deque<GameAction> ActionQueue;

/* Buffer for actions sent for future frames */
class MultiframeActionQueue {
public:
	bool extract_actions_for_frame(ActionQueue& actions, int frame);
	bool has_actions_for_frame(int frame);
	void queue_actions_for_frame(const ActionQueue& actions, int frame);
	void clear_actions();
	void clear();
private:
	std::map<int, ActionQueue> queue;
};

#endif /* ACTIONQUEUE_H_ */
