/*
 * lua_utils.h:
 *  Various lua utilities.
 */

#ifndef LUA_UTILS_H_
#define LUA_UTILS_H_

class LuaField;

/* Protect a lua table from returning nil. */
void lua_protect_table(const LuaField& field);

#endif /* LUA_UTILS_H_ */
