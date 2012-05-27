/*
 * levelgen.h
 *  Defines parameters for level & room generation as well as the generate_level & generate_rooms functions
 */
#ifndef LEVELGEN_H_
#define LEVELGEN_H_

#include "../util/mtwist.h"
#include "GeneratedLevel.h"
#include "itemgen.h"
#include "featuregen.h"
#include "tunnelgen.h"
#include "enemygen.h"

struct RoomGenSettings {
	int room_padding;
	int amount_of_rooms;
	int min_size, max_size;
	RoomGenSettings(int pad, int amnt, int mins, int maxs) :
		room_padding(pad), amount_of_rooms(amnt), min_size(mins), max_size(maxs) {
	}
};
struct LevelGenSettings {
	ItemGenSettings items;
	RoomGenSettings rooms;
	TunnelGenSettings tunnels;
	FeatureGenSettings features;
	EnemyGenSettings enemies;
	int level_w, level_h;

	LevelGenSettings(int w, int h, const ItemGenSettings& i,
			const RoomGenSettings& r, const TunnelGenSettings& t,
			const FeatureGenSettings& f, const EnemyGenSettings& e) :
			items(i), rooms(r), tunnels(t), features(f), enemies(e),
			level_w(w), level_h(h) {
	}
};

void generate_rooms(const RoomGenSettings& rs, MTwist& mt, GeneratedLevel& level);
void generate_level(const LevelGenSettings& ls, MTwist& mt, GeneratedLevel& level, GameState* gs);

#endif /* LEVELGEN_H_ */
