#ifndef LUA_BRIDGE_H_
#define LUA_BRIDGE_H_

struct lua_State;

// 5.1 backport of 5.2 stuff

#define lua_pushglobaltable(L)  \
		lua_pushvalue(L, LUA_GLOBALSINDEX)

/*
** convert an acceptable stack index into an absolute index
*/
int lua_absindex (struct lua_State *L, int idx);
void lua_rawgetp (struct lua_State *L, int idx, const void *p);
void lua_rawsetp (struct lua_State *L, int idx, const void *p);

#endif /* LUA_BRIDGE_H_ */
