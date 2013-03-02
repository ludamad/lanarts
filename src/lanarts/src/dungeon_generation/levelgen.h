/*
 * levelgen.h
 *  Defines parameters for level & room generation as well as the generate_level & generate_rooms functions
 */

#ifndef LEVELGEN_H_
#define LEVELGEN_H_

#include <lcommon/mtwist.h>

#include "dungeon_data.h"
#include "enemygen.h"
#include "featuregen.h"
#include "itemgen.h"
#include "tunnelgen.h"

class GameState;
class GameRoomState;
struct GeneratedRoom;
struct lua_State;

//return false on failure
bool generate_room_at(MTwist& mt, GeneratedRoom& level, const Region& r,
		int padding, int mark);
bool generate_room(MTwist& mt, GeneratedRoom& level, int rw, int rh,
		int padding, int mark, int max_attempts);

void generate_rooms(lua_State* L, const RegionGenSettings& rs, MTwist& mt,
		GeneratedRoom& level);
GameRoomState* generate_level(int roomid, MTwist& mt, GeneratedRoom& level,
		GameState* gs);

#endif /* LEVELGEN_H_ */
