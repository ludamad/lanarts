/*
 * EnemiesSeen.h:
 *  Keeps track of enemies that all players have seen
 */

#ifndef ENEMIESSEEN_H_
#define ENEMIESSEEN_H_

#include <set>

#include "../lanarts_defines.h"

class EnemiesSeen {
public:
	typedef std::set<enemy_id>::const_iterator iterator;
	EnemiesSeen() {
	}

	void mark_as_seen(enemy_id enemy) {
		enemies_seen.insert(enemy);
	}

	iterator begin() const {
		return enemies_seen.begin();
	}
	iterator end() const {
		return enemies_seen.end();
	}

	int amount() const {
		return enemies_seen.size();
	}

private:
	std::set<enemy_id> enemies_seen;
};

#endif /* ENEMIESSEEN_H_ */
