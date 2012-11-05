#include "SLB/lua_bridge.h"
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lstate.h"

// 5.1 backport of 5.2 stuff

/*
** convert an acceptable stack index into an absolute index
*/
LUA_API int lua_absindex (lua_State *L, int idx) {
  return (idx > 0 || idx <= LUA_REGISTRYINDEX)
         ? idx
         : cast_int(L->top - L->ci->func + idx);
}

LUA_API void lua_rawgetp (lua_State *L, int idx, const void *p) {

    idx = lua_absindex(L, idx);
    lua_pushlightuserdata(L, (void *) p);
    lua_rawget(L,idx);

}

LUA_API void lua_rawsetp (lua_State *L, int idx, const void *p) {

    idx = lua_absindex(L, idx);
    lua_pushlightuserdata(L, (void*) p);
    // put key behind value
    lua_insert(L, -2);
    lua_rawset(L,idx);
}
