/*
 * lua_geometry.h:
 *  Lua bindings for geometry structures
 */

#include "lua_geometry.h"

#include <luawrap/luawrap.h>
#include <luawrap/types.h>

#include "lua_numeric_tuple_helper.h"

template<typename T, typename V>
void install_numeric_tuple() {
	typedef LuaNumericTupleFunctions<T, V> ImplClass;
	luawrap::install_type<T, ImplClass::push, ImplClass::get, ImplClass::check>();
}

// lua state & module is not currently used but passed for future-proofing
void lua_register_geometry(lua_State* L, const LuaValue& module) {
	install_numeric_tuple<Pos, int>();
	install_numeric_tuple<Posf, float>();
	install_numeric_tuple<Dim, int>();
	install_numeric_tuple<DimF, float>();
	install_numeric_tuple<BBox, int>();
	install_numeric_tuple<BBoxF, float>();
}
