/*
 * lua_image.h:
 * 	Bindings for ldraw::Image
 */

#ifndef LUA_IMAGE_H_
#define LUA_IMAGE_H_

#include <common/lua/luacpp_wrap.h>
#include "../Image.h"

LUACPP_TYPE_WRAP(ldraw::Image, ldraw::lua_pushimage, ldraw::lua_getimage,
		ldraw::lua_checkimage);

class LuaValue;

namespace ldraw {
void lua_register_image(lua_State* L, const LuaValue& module);
}

#endif /* LUA_IMAGE_H_ */
