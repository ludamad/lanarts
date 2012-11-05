/*
 * EnemyEntry.h:
 *	Definitions for the enemy type, these are filled through the parsed yaml file.
 */

#ifndef ENEMYENTRY_H_
#define ENEMYENTRY_H_

#include <cstdlib>
#include <vector>
#include <string>
#include <cstdio>

#include "../../data/BaseDataEntry.h"

#include "../../lua/LuaValue.h"
#include "../../stats/combat_stats.h"

#include "../../lanarts_defines.h"

struct EnemyEntry: public BaseDataEntry {
	std::string appear_msg, defeat_msg;
	int radius, xpaward;
	sprite_id enemy_sprite, death_sprite;
	CombatStats basestats;
	bool unique;

	LuaValue init_event, step_event;

	EnemyEntry() :
			radius(15), xpaward(0), enemy_sprite(-1), death_sprite(-1), unique(
					false) {
	}
	virtual const char* entry_type() {
		return "Enemy";
	}
	virtual sprite_id get_sprite() {
		return enemy_sprite;
	}

	void init(lua_State* L) {
		init_event.initialize(L);
		step_event.initialize(L);
	}
};

extern std::vector<EnemyEntry> game_enemy_data;

enemy_id get_enemy_by_name(const char* name, bool error_if_not_found = true);

#endif /* ENEMYENTRY_H_ */
