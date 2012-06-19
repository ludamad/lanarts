/*
 * levelgen.h
 *  Defines parameters for level & room generation as well as the generate_level & generate_rooms functions
 */

#ifndef LEVELGEN_H_
#define LEVELGEN_H_

#include "../util/mtwist.h"

#include "itemgen.h"
#include "featuregen.h"
#include "tunnelgen.h"
#include "enemygen.h"

class GameState;
class GameLevelState;
struct GeneratedLevel;

struct RoomGenSettings {
	int room_padding;
	Range amount_of_rooms;
	Range size;
	RoomGenSettings() :
			room_padding(1), amount_of_rooms(0,0), size(0, 0) {

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

struct ContentGenSettings {
	ItemGenSettings items;
	FeatureGenSettings features;
	EnemyGenSettings enemies;
};

struct LevelGenSettings {
	std::vector<LayoutGenSettings> layouts;
	ContentGenSettings content;
};

void generate_rooms(const RoomGenSettings& rs, MTwist& mt,
		GeneratedLevel& level);
GameLevelState* generate_level(int roomid, MTwist& mt, GeneratedLevel& level,
		GameState* gs);

#endif /* LEVELGEN_H_ */
