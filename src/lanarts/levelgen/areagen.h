/*
 * areagen.h:
 *  Contains procedures related to generating a dungeon area
 */

//TODO: Rewrite pending here with tiered level generation

#ifndef AREAGEN_H_
#define AREAGEN_H_

#include <vector>
#include <common/lua/LuaValue.h>
#include "../lanarts_defines.h"

enum {
	DNGN_MAIN_BRANCH = 0
};

struct ItemGenSettings {
	itemgenlist_id itemgenlist;
	Range num_items;
};

struct TunnelGenSettings {
	int padding;
	std::string endpoint;
	Range size, num_tunnels;
	TunnelGenSettings() :
			padding(0) {

	}
};

struct RoomGenSettings {
	int room_padding;
	Range amount_of_rooms;
	Range size;
	RoomGenSettings() :
			room_padding(1), amount_of_rooms(0, 0), size(0, 0) {

	}
};

struct LayoutGenSettings {
	Range width, height;
	TunnelGenSettings tunnels;
	std::vector<RoomGenSettings> rooms;
	bool solid_fill;
	LayoutGenSettings() :
			solid_fill(true) {
	}
};

struct FeatureGenSettings {
	int nstairs_up, nstairs_down;
	std::vector<tileset_id> tilesets;
	FeatureGenSettings() :
			nstairs_up(3), nstairs_down(3) {
	}
};

struct EnemyGenChance {
	int guaranteed_spawns; //# of guaranteed generations of this enemy
	int generate_chance; //Proportion of chance to generate. Actual spawn chance relative to sum of all spawn chances.
	enemy_id enemytype;
	int generate_group_chance; //Chance that, if generating, generate group. Out of 100.
	Range groupsize;
};
struct EnemyGenSettings {
	std::vector<EnemyGenChance> enemy_chances;
	Range num_monsters;
	bool wandering;
	EnemyGenSettings() :
			wandering(true) {

	}
};

struct ContentGenSettings {
	ItemGenSettings items;
	FeatureGenSettings features;
	EnemyGenSettings enemies;
};

struct LevelGenSettings {
	std::vector<LayoutGenSettings> layouts;
	ContentGenSettings content;
	LuaValue gen_level_func;
};

struct DungeonBranch {
	LevelGenSettings* level_data;
	int nlevels;
	DungeonBranch() {
		level_data = NULL;
	}
	DungeonBranch(LevelGenSettings* level_data, int nlevels) :
			level_data(level_data), nlevels(nlevels) {
	}
};

#endif /* AREAGEN_H_ */
