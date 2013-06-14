/*
 * lua_ldungeon.h:
 *  Bindings for the ldungeon library to Lua.
 */

#ifndef LUA_LDUNGEON_H_
#define LUA_LDUNGEON_H_

struct lua_State;
class LuaValue;
class MTwist;

namespace ldungeon_gen {
	void lua_register_ldungeon(const LuaValue& module, MTwist* mtwist, bool register_lcommon);
}

#endif /* LUA_LDUNGEON_H_ */
