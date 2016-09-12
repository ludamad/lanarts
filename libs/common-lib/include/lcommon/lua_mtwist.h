/*
 * lua_mtwist.h:
 *  Registers the MTwist class for lua.
 */

#ifndef LUA_MTWIST_H_
#define LUA_MTWIST_H_

class LuaValue;
struct lua_State;

int luaopen_mtwist(lua_State* L);

#endif /* LUA_MTWIST_H_*/
