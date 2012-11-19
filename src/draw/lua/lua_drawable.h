/*
 * lua_drawable.h:
 *  Methods for lua <-> Drawable.
 *  Bound with a different metatable based on concrete type.
 */

#ifndef LRAW_LUA_DRAWABLE_H_
#define LRAW_LUA_DRAWABLE_H_

#include <common/lua/slb_valuetype.h>
#include "../Drawable.h"

SLB_WRAP_VALUE_TYPE(ldraw::Drawable, ldraw::lua_pushdrawable,
		ldraw::lua_getdrawable, ldraw::lua_checkdrawable);

namespace ldraw {
int luadrawablebase_index(lua_State* L, const DrawableBase& drawable,
		const char* member);
}

#endif /* LRAW_LUA_DRAWABLE_H_ */
