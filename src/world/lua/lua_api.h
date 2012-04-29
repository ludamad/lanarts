/*
 * lua_api.h
 *  Defines the lua api for interfacing with the game's elements
 */

#ifndef LUA_API_H_
#define LUA_API_H_
#include "../../util/game_basic_structs.h"

struct lua_State;
struct GameState;
struct ItemType;

obj_id lua_gameinst_arg(lua_State* lua_state, int narg);
ItemType& lua_item_arg(lua_State* lua_state, int narg);
void lua_pushitem(lua_State* lua_state, ItemType& item);
void lua_pushgameinst(lua_State* lua_state, obj_id id);
GameState* lua_get_gamestate(lua_State* lua_state);

void lua_gameinst_bindings(GameState* gs, lua_State* lua_state);
void lua_item_bindings(GameState* gs, lua_State* lua_state);
void lua_gamestate_bindings(GameState* gs, lua_State* lua_state);
void lua_lanarts_api(GameState* gs, lua_State* lua_state);

#endif /* LUA_API_H_ */
