/*
 * luaexpr.h:
 *  Function for calling a function and storing the result.
 */

#ifndef LUAEXPR_H_
#define LUAEXPR_H_

struct lua_State;
class LuaValue;

void luavalue_call_and_store(lua_State* L, LuaValue& eval);

#endif /* LUAEXPR_H_ */
