/*
 * lua_lsound.h:
 *  Bindings for the lsound component
 */

#ifndef LSOUND_LUA_LSOUND_H_
#define LSOUND_LUA_LSOUND_H_

struct LuaModule;
struct lua_State;

void lua_register_lsound(lua_State* L, const LuaModule& module);

#endif /* LSOUND_LUA_LSOUND_H_ */
