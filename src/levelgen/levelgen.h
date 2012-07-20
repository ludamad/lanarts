/*
 * levelgen.h
 *  Defines parameters for level & room generation as well as the generate_level & generate_rooms functions
 */

#ifndef LEVELGEN_H_
#define LEVELGEN_H_

#include "itemgen.h"
#include "featuregen.h"
#include "tunnelgen.h"
#include "enemygen.h"

#include "../data/dungeon_data.h"
#include "../util/mtwist.h"

class GameState;
class GameLevelState;
struct GeneratedLevel;

void generate_rooms(const RoomGenSettings& rs, MTwist& mt,
		GeneratedLevel& level);
GameLevelState* generate_level(int roomid, MTwist& mt, GeneratedLevel& level,
		GameState* gs);

#endif /* LEVELGEN_H_ */
