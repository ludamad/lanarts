/*
 * lua_gameinst.h:
 *  GameInst related lua routines
 */

#ifndef LUA_GAMEINST_H_
#define LUA_GAMEINST_H_

struct lua_State;
class LuaValue;
class GameInst;

void lua_gameinst_callback(lua_State* L, LuaValue& value, GameInst* inst);

void lua_register_gameinst(lua_State* L);

#endif /* LUA_GAMEINST_H_ */