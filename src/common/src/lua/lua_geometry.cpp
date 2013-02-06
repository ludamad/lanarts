/*
 * lua_geometry.h:
 *  Lua bindings for geometry structures
 */

#include "lua_geometry.h"

#include <luawrap/luawrap.h>
#include <luawrap/types.h>
#include <luawrap/functions.h>

#include "lua_numeric_tuple_helper.h"

template<typename T, typename V>
static void install_numeric_tuple() {
	typedef LuaNumericTupleFunctions<T, V> ImplClass;
	luawrap::install_type<T, ImplClass::push, ImplClass::get, ImplClass::check>();
}

static bool bbox_contains(const BBoxF& bbox, const Posf& pos) {
	return bbox.contains(pos);
}
static BBoxF bbox_create(const Posf& pos, const SizeF& size) {
	return BBoxF(pos, size);
}

// lua state & module is not currently used but passed for future-proofing
void lua_register_geometry(lua_State* L, const LuaValue& module) {
	install_numeric_tuple<Pos, int>();
	install_numeric_tuple<Posf, float>();
	install_numeric_tuple<Size, int>();
	install_numeric_tuple<SizeF, float>();
	install_numeric_tuple<BBox, int>();
	install_numeric_tuple<BBoxF, float>();
	module["bbox_contains"].bind_function(bbox_contains);
	module["bbox_create"].bind_function(bbox_create);
}
