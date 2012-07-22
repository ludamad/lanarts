/*
 * dungeon_data.h:
 *  Defines dungeon room generation settings
 */

#ifndef DUNGEON_DATA_H_
#define DUNGEON_DATA_H_

#include <vector>
#include "../lua/LuaValue.h"
#include "../lanarts_defines.h"

enum {
	DNGN_MAIN_BRANCH = 0
};

struct ItemGenChance {
	int genchance; //Out of 100%
	int itemtype;
	Range quantity;
};
struct ItemGenSettings {
	std::vector<ItemGenChance> item_chances;
	Range num_items;
};

struct TunnelGenSettings {
	int padding;
	Range size, num_tunnels;
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

extern DungeonBranch game_dungeon_data[];
extern std::vector<LevelGenSettings> game_dungeon_yaml;
extern size_t game_dungeon_n;

#endif /* DUNGEON_DATA_H_ */
