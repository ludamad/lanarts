/*
 * lua_image.h:
 * 	Bindings for ldraw::Image
 */

#ifndef LUA_IMAGE_H_
#define LUA_IMAGE_H_

#include "../Image.h"

struct lua_State;

void lua_register_image(lua_State* L);

#endif /* LUA_IMAGE_H_ */
