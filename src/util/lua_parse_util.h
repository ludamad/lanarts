/*
 * lua_parse_util.h:
 *  Generic helpers for parsing via LuaField
 */

#ifndef LUA_PARSE_UTIL_H_
#define LUA_PARSE_UTIL_H_

class LuaField;

namespace lua_util {

const LuaField& make_pair_if_num(const LuaField& field);

}

#endif /* LUA_PARSE_UTIL_H_ */
