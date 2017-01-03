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

#include <lcommon/LuaLazyValue.h>

#include "data/ResourceEntryBase.h"

#include "stats/combat_stats.h"

#include "lanarts_defines.h"

struct EnemyEntry: public ResourceEntryBase {
	std::string appear_msg, defeat_msg;
	int radius = 15, xpaward = 0;
	sprite_id enemy_sprite = -1, death_sprite = -1;
	CombatStats basestats;
	bool unique = false;
	std::vector<effect_id> active_effects;
	int vision_radius = 7; // TODO, use

	LuaLazyValue init_event, step_event, draw_event, death_event;
	StatusEffectModifiers effect_modifiers;

	void init(lua_State* L) {
		init_event.initialize(L);
		step_event.initialize(L);
		draw_event.initialize(L);
	}
	virtual const char* entry_type() {
		return "Enemy";
	}
	virtual sprite_id get_sprite() {
		return enemy_sprite;
	}
};

extern std::vector<EnemyEntry> game_enemy_data;

enemy_id get_enemy_by_name(const char* name, bool error_if_not_found = true);

#endif /* ENEMYENTRY_H_ */
