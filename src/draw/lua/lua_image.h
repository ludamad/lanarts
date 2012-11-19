/*
 * lua_image.h:
 * 	Bindings for ldraw::Image
 */

#ifndef LUA_IMAGE_H_
#define LUA_IMAGE_H_

#include <common/lua/slb_valuetype.h>
#include "../Image.h"

struct lua_State;

SLB_WRAP_VALUE_TYPE(ldraw::Image, ldraw::lua_pushimage, ldraw::lua_getimage, ldraw::lua_checkimage);

#endif /* LUA_IMAGE_H_ */
