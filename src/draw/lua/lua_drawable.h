/*
 * lua_drawable.h:
 *  Methods for lua <-> Drawable.
 *  Bound with a different metatable based on concrete type.
 */

#ifndef LRAW_LUA_DRAWABLE_H_
#define LRAW_LUA_DRAWABLE_H_

#include <common/lua/luacpp_wrap.h>
#include "../Drawable.h"

class LuaValue;

LUACPP_TYPE_WRAP(ldraw::Drawable, ldraw::lua_pushdrawable,
		ldraw::lua_getdrawable, ldraw::lua_checkdrawable);

namespace ldraw {
int luadrawablebase_index(lua_State* L, const DrawableBase& drawable,
		const char* member);

// Register assorted drawable helper functions:
// directional_create, drawable_create, animation_create
void lua_register_drawables(lua_State* L, const LuaValue& module);

}

#endif /* LRAW_LUA_DRAWABLE_H_ */
