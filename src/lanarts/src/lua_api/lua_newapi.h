/*
 * lua_newapi.h:
 *  New API, rename to lua_api once completed
 */

#ifndef LUA_NEWAPI_H_
#define LUA_NEWAPI_H_

class GameState;
struct lua_State;
class LuaStackValue;
class LuaValue;

namespace lua_api {
	// Grab the GameState back-pointer
	// Used to implement lua_api functions that affect GameState
	GameState* gamestate(lua_State* L);
	// Convenience function that performs above on captured lua state
	GameState* gamestate(const LuaStackValue& val);

	void preinit_state(lua_State* L); // TODO: This should be removed some time

	void register_io_api(lua_State* L);

	/* Register general utility functions */
	void register_general_api(lua_State* L) ;

	/* Functions for visual results in the lanarts world, eg drawing text */
	void register_display_api(lua_State* L);

	// Special-case functions such as save_game, load_game, regen_level.
	// Some of these are for testing purposes only.
	void register_gamestate_api(lua_State* L);

	// Register the lanarts level and world querying API
	void register_gameworld_api(lua_State* L);

	// Register the lanarts networking API
	void register_net_api(lua_State* L);

	// Register all the lanarts API functions and types
	void register_api(GameState* gs, lua_State* L);

	// Callbacks into the lua VM:
	void luacall_main(lua_State* L);
	void luacall_postdraw(lua_State* L);
}


#endif /* LUA_NEWAPI_H_ */
