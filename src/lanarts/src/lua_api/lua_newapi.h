/*
 * lua_newapi.h:
 *  New API, rename to lua_api once completed
 */

#ifndef LUA_NEWAPI_H_
#define LUA_NEWAPI_H_

class GameState;
class GameInst;
class PlayerInst;
class ProjectileInst;
class EnemyInst;
struct lua_State;
class LuaStackValue;
class LuaValue;
union SDL_Event;

namespace lua_api {
	// Grab the GameState back-pointer
	// Used to implement lua_api functions that affect GameState
	GameState* gamestate(lua_State* L);
	// Convenience function that performs above on captured lua state
	GameState* gamestate(const LuaStackValue& val);

	LuaValue global_getters(lua_State* L);
	LuaValue global_setters(lua_State* L);
	void globals_set_mutability(lua_State* L, bool mutability);
	bool globals_get_mutability(lua_State* L);

	/* Add a path for searching with the 'require' function. */
	void add_search_path(lua_State* L, const char* path);
	/* Use serach path to load a module if it has not yet been loaded. */
	void require(lua_State* L, const char* path);

	/* Creates a lua state with a custom global metatable.
	 * All further registration assumes the lua state was created with this function. */
	lua_State* create_luastate();

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
	bool luacall_handle_event(lua_State* L, SDL_Event* e);
	void luacall_overlay_draw(lua_State* L);
	void luacall_post_draw(lua_State* L);
	void luacall_hitsound(lua_State* L);

	void event_player_death(lua_State* L, PlayerInst* player);
	void event_monster_death(lua_State* L, EnemyInst* player);
	void event_projectile_hit(lua_State* L, ProjectileInst* projectile, GameInst* target);

	void luacall_game_won(lua_State* L);

	// Lua utilities:
	int l_itervalues(lua_State* L);
}


#endif /* LUA_NEWAPI_H_ */
