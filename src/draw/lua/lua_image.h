/*
 * lua_image.h:
 * 	Bindings for ldraw::Image
 */

#ifndef LDRAW_LUA_IMAGE_H_
#define LDRAW_LUA_IMAGE_H_

#include "../Image.h"
#include <common/lua/luacpp_wrap.h>

LUACPP_TYPE_WRAP(ldraw::Image,
		ldraw::lua_pushimage, ldraw::lua_getimage,
		ldraw::lua_checkimage);


class LuaValue;

namespace ldraw {
void lua_register_image(lua_State* L, const LuaValue& module);
}

#endif /* LDRAW_LUA_IMAGE_H_ */
