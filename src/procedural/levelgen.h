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
	int amount_of_rooms;
	int min_size, max_size;
	bool solid_fill;
	RoomGenSettings(int pad, int amnt, int mins, int maxs, bool solid_fill) :
			room_padding(pad), amount_of_rooms(amnt), min_size(mins), max_size(
					maxs), solid_fill(solid_fill) {
	}
};
struct LevelGenSettings {
	ItemGenSettings items;
	RoomGenSettings rooms;
	TunnelGenSettings tunnels;
	FeatureGenSettings features;
	EnemyGenSettings enemies;
	int level_w, level_h;
	bool wander;

	LevelGenSettings(int w, int h, bool wander, const ItemGenSettings& i,
			const RoomGenSettings& r, const TunnelGenSettings& t,
			const FeatureGenSettings& f, const EnemyGenSettings& e) :
			items(i), rooms(r), tunnels(t), features(f), enemies(e), level_w(w), level_h(
					h), wander(wander) {
	}
};

void generate_rooms(const RoomGenSettings& rs, MTwist& mt,
		GeneratedLevel& level);
GameLevelState* generate_level(int roomid, MTwist& mt, GeneratedLevel& level,
		GameState* gs);

#endif /* LEVELGEN_H_ */
