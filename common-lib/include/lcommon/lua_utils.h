/*
 * lua_utils.h:
 *  Various lua utilities.
 */

#ifndef LUA_UTILS_H_
#define LUA_UTILS_H_

class LuaValue;
class LuaField;

/* Protect a lua table from returning nil. */
void lua_protect_table(const LuaField& field);

/* Ensures a table exists here, and create a protected table if not */
LuaValue lua_ensure_protected_table(const LuaField& field);

#endif /* LUA_UTILS_H_ */
