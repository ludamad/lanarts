/*
 * lua_api.h
 *  Defines the lua api for interfacing with the game's elements
 */

#ifndef LUA_API_H_
#define LUA_API_H_
#include "../util/game_basic_structs.h"

struct lua_State;
class GameState;
class GameInst;
struct ItemEntry;
struct EffectEntry;
struct CombatStats;
struct EffectiveStats;
struct EffectiveAttackStats;

GameInst* lua_gameinst_arg(lua_State* L, int narg);
void lua_push_gameinst(lua_State* L, GameInst* inst);

void lua_push_combatstats(lua_State* L, GameInst* inst);
void lua_push_combatstats(lua_State* L, const CombatStats& stats);
void lua_push_effectivestats(lua_State* L, const EffectiveStats& stats);
void lua_push_effectiveattackstats(lua_State* L, const EffectiveAttackStats& stats);

CombatStats& lua_get_combatstats(lua_State* L, int narg);
EffectiveStats& lua_get_effectivestats(lua_State* L, int narg);

GameState* lua_get_gamestate(lua_State* L);

void lua_lanarts_api(GameState* gs, lua_State* L);

//Implemented in data/game_data.cpp
void luayaml_push_item(lua_State *L, const char* name);
void luayaml_push_sprites(lua_State *L, const char* name);
void luayaml_push_enemies(lua_State *L, const char* name);
void luayaml_push_levelinfo(lua_State *L, const char* name);

#endif /* LUA_API_H_ */
