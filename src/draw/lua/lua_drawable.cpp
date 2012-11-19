/*
 * lua_drawable.cpp:
 *  Methods for lua <-> Drawable.
 *  Bound with a different metatable based on concrete type.
 */

extern "C" {
#include <lua/lua.h>
}

#include <new>

#include <common/lua/luacpp.h>
#include <common/lua/lua_geometry.h>

#include "../DrawableBase.h"
#include "../ldraw_assert.h"
#include "../Image.h"

#include "lua_drawable.h"
#include "lua_drawoptions.h"

namespace ldraw {

void lua_pushdrawable(lua_State* L, const Drawable& drawable) {
	Drawable* luadrawable = (Drawable*)lua_newuserdata(L, sizeof(Drawable));
	//Construct a drawable in this location:
	new (luadrawable) Drawable(drawable);
	drawable.get_ref()->push_metatable(L);
	lua_setmetatable(L, -2);
}

const Drawable& lua_getdrawable(lua_State* L, int idx) {
	Drawable* luadrawable = (Drawable*)lua_touserdata(L, idx);
	return *luadrawable;
}

bool lua_checkdrawable(lua_State* L, int idx) {
	int type = lua_type(L, idx);
	if (type != LUA_TUSERDATA && type != LUA_TFUNCTION) {
		return false;
	}
	//TODO: Figure out a way to check for the various metatables
	return true;
}

static int luacfunc_draw(lua_State* L) {
	using namespace ldraw;
	using namespace SLB;

	const char* LERR_MSG =
			"Incorrect Image::draw usage, use img:draw(position) or img:draw(options, position)";

	int nargs = lua_gettop(L);

	if (!lua_checkdrawable(L, 1)) {
		luaL_error(L, LERR_MSG);
		return 0;
	}
	const Drawable& drawable = lua_getdrawable(L, 1);

	if (nargs == 2) {
		Posf p = get<Posf>(L, 2);
		drawable.draw(p);
	} else if (nargs == 3) {
		DrawOptions options = luacpp_get<DrawOptions>(L, 2);
		Posf p = get<Posf>(L, 3);
		drawable.draw(options, p);
	} else {
		luaL_error(L, LERR_MSG);
	}

	return 0;
}

//Metatable implementation
static const char DRAWABLEBASE[] = "LDraw::DrawableBase";

static void drawable_base_push_metatable(lua_State* L);

int luadrawablebase_index(lua_State* L, const DrawableBase& drawable,
		const char* member) {
	if (strcmp(member, "duration") == 0) {
		float duration = drawable.animation_duration();
		if (duration == 0.0f) {
			lua_pushnil(L);
		} else {
			lua_pushnumber(L, duration);
		}
	} else if (strcmp(member, "animated") == 0) {
		lua_pushboolean(L, drawable.is_animated());
	} else {
		drawable_base_push_metatable(L);
		lua_getfield(L, -1, member);
		lua_replace(L, -2);
	}

	return 1;
}

static int luadrawable_index(lua_State* L) {
	using namespace SLB;

	if (!lua_checkdrawable(L, 1)) {
		luaL_error(L,
				"Error indexing supposed Drawable object -- not a Drawable.");
		return 0;
	}
	const Drawable& drawable = lua_getdrawable(L, 1);
	const char* member = lua_tostring(L, 2);

	return luadrawablebase_index(L, *drawable.get_ref().get(), member);
}

static void drawable_base_push_metatable(lua_State* L) {
	lua_getfield(L, LUA_REGISTRYINDEX, DRAWABLEBASE);

	if (!lua_isnil(L, -1)) {
		return; // Cached table is pushed
	}lua_pop(L, 1);
	//pop nil

	luaL_newmetatable(L, DRAWABLEBASE);

	int metatable = lua_gettop(L);
	lua_pushcfunction(L, luadrawable_index);
	lua_setfield(L, metatable, "__index"); /*Set self as index*/
	lua_pushcfunction(L, luacfunc_draw);
	lua_setfield(L, metatable, "draw");
}

void DrawableBase::push_metatable(lua_State* L) const {
	drawable_base_push_metatable(L);
}

}
