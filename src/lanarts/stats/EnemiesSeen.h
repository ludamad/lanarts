/*
 * EnemiesSeen.h:
 *  Keeps track of enemies that all players have seen
 */

#ifndef ENEMIESSEEN_H_
#define ENEMIESSEEN_H_

#include <set>

#include <common/SerializeBuffer.h>

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
	void serialize(SerializeBuffer& serializer) {
		serializer.write_int(amount());
		for (iterator it = begin(); it != end(); it++) {
			serializer.write(*it);
		}
	}
	void deserialize(SerializeBuffer& serializer) {
		enemies_seen.clear();
		int amnt;
		serializer.read_int(amnt);
		for (int i = 0; i < amnt; i++) {
			enemy_id id;
			serializer.read(id);
			enemies_seen.insert(id);
		}
	}

private:
	std::set<enemy_id> enemies_seen;
};

#endif /* ENEMIESSEEN_H_ */
