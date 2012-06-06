/*
 * enemy_data.h
 *	Definitions for the enemy type, these are filled through the parsed yaml file.
 */

#ifndef ENEMY_DATA_H_
#define ENEMY_DATA_H_
#include <cstdlib>
#include <vector>
#include <string>
#include <cstdio>

#include "../util/LuaValue.h"
#include "../gamestats/Stats.h"

struct EnemyEntry {
	std::string name;
	int radius;
	int xpaward;
	int sprite_number;
	Stats basestats;
	bool unique;

	LuaValue init_event, step_event;

	EnemyEntry() {
	}
	EnemyEntry(const std::string& name, int rad, int xpaward, int spriten,
			const Stats& stats, const std::string& initev,
			const std::string& stepev, bool unique = false) :
			name(name), radius(rad), xpaward(xpaward), sprite_number(spriten), basestats(
					stats), init_event(initev), step_event(stepev), unique(unique) {
	}

	void init(lua_State* L) {
		init_event.initialize(L);
		step_event.initialize(L);
	}
};

extern std::vector<EnemyEntry> game_enemy_data;

#endif /* ENEMY_DATA_H_ */
