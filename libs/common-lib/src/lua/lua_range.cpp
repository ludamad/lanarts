/*
 * lua_range.cpp:
 *  Lua binding for range
 */

#include "lua_range.h"

#include <luawrap/luawrap.h>
#include <luawrap/types.h>

#include "lua_numeric_tuple_helper.h"

template<typename T, typename V>
void install_numeric_tuple() {
	typedef LuaNumericTupleFunctions<T, V> ImplClass;
	luawrap::install_type<T, ImplClass::push, ImplClass::get, ImplClass::check>();
}

// lua state & module is not currently used but passed for future-proofing
void lua_register_range(lua_State* L, const LuaValue& module) {
	install_numeric_tuple<Range, int>();
	install_numeric_tuple<RangeF, float>();
}
