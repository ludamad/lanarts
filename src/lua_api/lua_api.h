/*
 * lua_api.h:
 *  C++-side of the Lua API. Defines most of the 'core' module.
 */

#ifndef LUA_API_H_
#define LUA_API_H_

class GameState;
class GameInst;
class PlayerInst;
class ProjectileInst;
class EnemyInst;

struct lua_State;
class LuaStackValue;
class LuaValue;
class LuaField;
struct LuaModule;
union SDL_Event;

namespace lua_api {
	// Grab the GameState back-pointer
	// Used to implement lua_api functions that affect GameState
	GameState* gamestate(lua_State* L);
	// Convenience function that performs above on captured lua state
	GameState* gamestate(const LuaStackValue& val);
	void register_gamestate(GameState* gs, lua_State* L);


	/* Add a path for searching with the 'require' function. */
	void add_search_path(lua_State* L, const char* path);
	/* Use serach path to load a module if it has not yet been loaded. */
	void require(lua_State* L, const char* path, bool keep_return = false);

	/* Creates a lua state with a custom global metatable.
	 * All further registration assumes the lua state was created with this function. */
	lua_State* create_configured_luastate();

	void preinit_state(lua_State* L); // TODO: This should be removed some time

	void register_io_api(lua_State* L);

	void register_lua_submodule(lua_State* L, const char* vpath, LuaValue module);
	LuaValue register_lua_submodule(lua_State* L, const char* vpath);
	LuaModule register_lua_submodule_as_luamodule(lua_State* L, const char* vpath);

	/* Register general utility functions */
	void register_general_api(lua_State* L) ;

	/* Functions for the event log, namely adding messages */
	void register_event_log_api(lua_State* L);

	void register_tiles_api(lua_State* L);

	// Special-case functions such as save_game, load_game, regen_level.
	// Some of these are for testing purposes only.
	void register_gamestate_api(lua_State* L);

	// Register the lanarts level and world querying API
	void register_gameworld_api(lua_State* L);

	// Register the lanarts networking API
	void register_net_api(lua_State* L);

	void register_lua_libraries(lua_State* L);
	// Register all the lanarts API functions and types
	void register_api(GameState* gs, lua_State* L);

	// Register internal engine functions for setting up Lanarts state
	void register_lua_core_EngineInternal(lua_State* L);

	void pretty_print(LuaField field);
	LuaValue import(lua_State* L, const char* virtual_path);

	// Callbacks into the lua VM:
	bool luacall_handle_event(lua_State* L, SDL_Event* e);
	void luacall_overlay_draw(lua_State* L);
	void luacall_post_draw(lua_State* L);
	void luacall_hitsound(lua_State* L);

	void event_player_init(lua_State* L, PlayerInst* player);
        // Returns whether the game should end:
	bool event_player_death(lua_State* L, PlayerInst* player);
	void event_monster_init(lua_State* L, EnemyInst* enemy);
	void event_monster_death(lua_State* L, EnemyInst* enemy);
	void event_projectile_hit(lua_State* L, ProjectileInst* projectile, GameInst* target);

	void luacall_game_won(lua_State* L);

	// Lua utilities:
	int l_itervalues(lua_State* L);
}

// OLD API BRIDGE:
// TODO: Remove (almost) obsolete methods

class CombatStats;
class EffectiveStats;
class EffectiveAttackStats;

void lua_push_effectivestats(lua_State* L, GameInst* inst);
void lua_push_effectivestats(lua_State* L, const EffectiveStats& stats);
void lua_push_effectiveattackstats(lua_State* L, const EffectiveAttackStats& stats);
EffectiveAttackStats lua_pop_effectiveattackstats(lua_State *L);
EffectiveStats& lua_get_effectivestats(lua_State* L, int idx);

void lua_spelltarget_bindings(lua_State* L);
void lua_effectivestats_bindings(GameState* gs, lua_State* L);
void lua_combatstats_bindings(GameState* gs, lua_State* L);

void lua_push_combatstats(lua_State* L, GameInst* inst);
void lua_push_combatstats(lua_State* L, const CombatStats& stats);

void luayaml_push_item(lua_State* L, const char* name);

#endif /* LUA_API_H_ */
