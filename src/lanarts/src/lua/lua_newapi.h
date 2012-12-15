/*
 * lua_newapi.h:
 *  New API, rename to lua_api once completed
 */

#ifndef LUA_NEWAPI_H_
#define LUA_NEWAPI_H_

class GameState;
struct lua_State* L;

namespace lua_api {
	// Grab the GameState back-pointer
	// Used to implement lua_api functions that affect GameState
	GameState* gamestate(lua_State* L);

	// Register the lanarts API functions and types
	void register_api(GameState* gs, lua_State* L);
}


#endif /* LUA_NEWAPI_H_ */
