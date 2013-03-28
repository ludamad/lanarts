/*
 * lua_parse_util.cpp:
 *  Generic helpers for parsing via LuaField
 */

#include <luawrap/luawrap.h>
#include "lua_parse_util.h"

const LuaField& lua_util::make_pair_if_num(const LuaField& field) {
	using namespace luawrap;
	if (field.is<double>()) {
		double num = field.to_num();
		field.newtable();
		field[1] = num;
		field[2] = num;
	}
	return field;
}
