/*
 * lua_image.cpp:
 * 	Bindings for ldraw::Image
 * 	A specialization of the bindings for Drawable
 */

#include <cstring>
#include <new>

#include <lua.hpp>

#include <luawrap/LuaValue.h>
#include <luawrap/functions.h>
#include <luawrap/types.h>

#include "DrawableBase.h"
#include "ldraw_assert.h"
#include "Image.h"

#include "lua_drawable.h"
#include "lua_drawoptions.h"
#include "lua_image.h"

namespace ldraw {

void lua_pushimage(lua_State* L, const Image& image) {
	Drawable drawable(new Image(image));
	lua_pushdrawable(L, drawable);
}

const Image& lua_getimage(lua_State* L, int idx) {
	Image* img = dynamic_cast<Image*>(lua_getdrawable(L, idx).get_ref().get());
	LDRAW_ASSERT(img);
	return *img;
}

Image lua_getimage2(lua_State* L, int idx) {
	return lua_getimage(L, idx);
}

bool lua_checkimage(lua_State* L, int idx) {
	if (lua_type(L, idx) != LUA_TUSERDATA) {
		return false;
	}
	return dynamic_cast<Image*>(lua_getdrawable(L, idx).get_ref().get());
}

static const char IMAGE[] = "LDraw::Image";
static int luaimage_index(lua_State *L) {
	using namespace ldraw;

	if (!lua_checkimage(L, 1)) {
		luaL_error(L, "Error indexing supposed Image object -- not an image.");
		return 0;
	}
	const Image & image = lua_getimage(L, 1);
	const char* member = lua_tostring(L, 2);

	//TODO Evaluate if this is still needed, all it does is forward
	return luadrawablebase_index(L, image, member);
}

void Image::push_metatable(lua_State *L) const {
	lua_getfield(L, LUA_REGISTRYINDEX, IMAGE);
	if (!lua_isnil(L, -1)) {
		return; // Cached table is pushed
	}
	lua_pop(L, 1);
	luaL_newmetatable(L, IMAGE);
	int metatable = lua_gettop(L);
	lua_pushcfunction(L, luaimage_index);
	lua_setfield(L, metatable, "__index");
}

static int image_load(lua_State *L) {
	int nargs = lua_gettop(L);
	bool rotatable = false;

	if (nargs < 1 || nargs > 2) {
		luaL_error(L, "Invalid usage of image_load(filename [, rotatable])");
	}

	if (nargs == 2) {
		rotatable = lua_toboolean(L, 2);
	}

	lua_pushimage(L, Image(lua_tostring(L, 1), BBoxF(), rotatable));
	return 1;
}

void lua_register_image(lua_State *L, const LuaValue & module) {
	luawrap::install_type<ldraw::Image, lua_pushimage, lua_getimage2,
			lua_checkimage>();

	module["image_load"].bind_function(image_load);
	module["image_split"].bind_function(image_split);
}

}

