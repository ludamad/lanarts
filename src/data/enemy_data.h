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

#include "../gamestats/combat_stats.h"

struct EnemyEntry {
	std::string name, appear_msg, defeat_msg;
	int radius;
	int xpaward;
	int sprite_number;
	CombatStats basestats;
	bool unique;

	LuaValue init_event, step_event;

	EnemyEntry() :
			radius(15), xpaward(0), sprite_number(-1), unique(false) {
	}

	void init(lua_State* L) {
		init_event.initialize(L);
		step_event.initialize(L);
	}
};

extern std::vector<EnemyEntry> game_enemy_data;

int get_enemy_by_name(const char* name);

#endif /* ENEMY_DATA_H_ */
