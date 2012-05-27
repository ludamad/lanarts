/*
 * lua_api.h
 *  Defines the lua api for interfacing with the game's elements
 */

#ifndef LUA_API_H_
#define LUA_API_H_
#include "../../util/game_basic_structs.h"

struct lua_State;
struct GameState;
struct ItemEntry;
struct EffectType;

obj_id lua_gameinst_arg(lua_State* L, int narg);
ItemEntry& lua_item_arg(lua_State* L, int narg);
EffectType& lua_effects_arg(lua_State* L, int narg);
void lua_pushitem(lua_State* L, ItemEntry& item);
void lua_pushgameinst(lua_State* L, obj_id id);
void lua_pusheffects(lua_State* L, int effectnum);
GameState* lua_get_gamestate(lua_State* L);

void lua_gameinst_bindings(GameState* gs, lua_State* L);
void lua_item_bindings(GameState* gs, lua_State* L);
void lua_effects_bindings(GameState* gs, lua_State* L);
void lua_gamestate_bindings(GameState* gs, lua_State* L);
void lua_lanarts_api(GameState* gs, lua_State* L);

//Implemented in data/game_data.cpp
void luayaml_push_item(lua_State *L, const char* name);
void luayaml_push_sprites(lua_State *L, const char* name);
void luayaml_push_enemies(lua_State *L, const char* name);
void luayaml_push_levelinfo(lua_State *L, const char* name);

#endif /* LUA_API_H_ */
