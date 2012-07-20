/*
 * ActionQueue.h:
 *  Provides ActionQueue class, which is an alias for a deque.
 */

#ifndef ACTIONQUEUE_H_
#define ACTIONQUEUE_H_

#include <deque>

#include "../gamestate/GameAction.h"

typedef std::deque<GameAction> ActionQueue;

#endif /* ACTIONQUEUE_H_ */
