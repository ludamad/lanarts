/*
 * lua_luadrawable.h:
 * 	Bindings for ldraw::LuaDrawable
 */

#ifndef LUA_LUADRAWABLE_H_
#define LUA_LUADRAWABLE_H_

#include <common/lua/luacpp_wrap.h>
#include "../LuaDrawable.h"

LUACPP_TYPE_WRAP(ldraw::LuaDrawable, ldraw::lua_pushluadrawable, ldraw::lua_getluadrawable,
		ldraw::lua_checkluadrawable);

#endif /* LUA_LUADRAWABLE_H_ */
