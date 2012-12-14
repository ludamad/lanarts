/*
 * levelgen.h
 *  Defines parameters for level & room generation as well as the generate_level & generate_rooms functions
 */

#ifndef LEVELGEN_H_
#define LEVELGEN_H_

#include <common/mtwist.h>
#include "dungeon_data.h"
#include "enemygen.h"
#include "featuregen.h"

#include "itemgen.h"
#include "tunnelgen.h"

class GameState;
class GameLevelState;
struct GeneratedLevel;
struct lua_State;

//return false on failure
bool generate_room_at(MTwist& mt, GeneratedLevel& level, const Region& r,
		int padding, int mark);
bool generate_room(MTwist& mt, GeneratedLevel& level, int rw, int rh,
		int padding, int mark, int max_attempts);

void generate_rooms(lua_State* L, const RoomGenSettings& rs, MTwist& mt,
		GeneratedLevel& level);
GameLevelState* generate_level(int roomid, MTwist& mt, GeneratedLevel& level,
		GameState* gs);

#endif /* LEVELGEN_H_ */
