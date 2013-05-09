/*
 * lua_drawable.cpp:
 *  Methods for lua <-> Drawable.
 *  Bound with a different metatable based on concrete type.
 */

#include <cstring>

#include <lua.hpp>

#include <stdexcept>
#include <new>

#include <luawrap/LuaValue.h>
#include <luawrap/luawrap.h>
#include <luawrap/types.h>
#include <luawrap/functions.h>

#include "Animation.h"
#include "DrawableBase.h"
#include "DirectionalDrawable.h"
#include "LuaDrawable.h"
#include "ldraw_assert.h"
#include "Image.h"

#include "lua_drawable.h"
#include "lua_drawoptions.h"

namespace ldraw {

void lua_pushdrawable(lua_State* L, const Drawable& drawable) {
	Drawable* luadrawable = (Drawable*)((lua_newuserdata(L,
				sizeof(Drawable))));
		new (luadrawable) Drawable(drawable);
		drawable.get_ref()->push_metatable(L);
		lua_setmetatable(L, -2);
	}

	Drawable lua_getdrawable(lua_State* L, int idx) {
		if (lua_isfunction(L, idx)) {
			return Drawable(new LuaDrawable(L, LuaValue(L, idx)));
		} else if (!lua_isuserdata(L, idx)) {
			throw std::runtime_error(
					"Attempt to pass incompatible type as drawable");
		}

		Drawable* luadrawable = (Drawable*)((lua_touserdata(L, idx)));
		return *luadrawable;
	}

	bool lua_checkdrawable(lua_State* L, int idx) {
		int type = lua_type(L, idx);
		if (type != LUA_TUSERDATA && type != LUA_TFUNCTION) {
			return false;
		}
		return true;
	}

	static int luacfunc_draw(lua_State* L) {
		using namespace ldraw;
		using namespace luawrap;
		const char* LERR_MSG =
				"Incorrect Image::draw usage, use img:draw(position) or img:draw(options, position)";
		int nargs = lua_gettop(L);
		if (!lua_checkdrawable(L, 1)) {
			luaL_error(L, LERR_MSG);
			return 0;
		}
		const Drawable& drawable = lua_getdrawable(L, 1);
		if (nargs == 2) {
			PosF p = get < PosF > (L, 2);
			drawable.draw(p);
		} else if (nargs == 3) {
			DrawOptions options = luawrap::get < DrawOptions > (L, 2);
			PosF p = get < PosF > (L, 3);
			drawable.draw(options, p);
		} else {
			luaL_error(L, LERR_MSG);
		}

		return 0;
	}

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
		if (!lua_checkdrawable(L, 1)) {
			luaL_error(L,
					"Error indexing supposed Drawable object -- not a Drawable.");
			return 0;
		}
		const Drawable& drawable = lua_getdrawable(L, 1);
		const char* member = lua_tostring(L, 2);
		return luadrawablebase_index(L, *drawable.ptr(), member);
	}

	static void drawable_base_push_metatable(lua_State* L) {
		lua_getfield(L, LUA_REGISTRYINDEX, DRAWABLEBASE);
		if (!lua_isnil(L, -1)) {
			return; // Cached table is pushed
		}
		lua_pop(L, 1);
		luaL_newmetatable(L, DRAWABLEBASE);
		int metatable = lua_gettop(L);
		lua_pushcfunction(L, luadrawable_index);
		lua_setfield(L, metatable, "__index");
		lua_pushcfunction(L, luacfunc_draw);
		lua_setfield(L, metatable, "draw");
	}

	void DrawableBase::push_metatable(lua_State* L) const {
		drawable_base_push_metatable(L);
	}

	static Drawable directional_create(const std::vector<Drawable>& directions,
			float angle_offset) {
		return Drawable(new DirectionalDrawable(directions, angle_offset));
	}

	static Drawable animation_create(const std::vector<Drawable>& frames,
			float animation_speed) {
		return Drawable(new Animation(frames, animation_speed));
	}

	void lua_pushluadrawable(lua_State* L, const LuaDrawable& image) {
		lua_pushdrawable(L, Drawable(new LuaDrawable(image)));
	}

	LuaDrawable lua_getluadrawable(lua_State* L, int idx) {
		return *dynamic_cast<LuaDrawable*>(lua_getdrawable(L, idx).ptr());
	}

	bool lua_checkluadrawable(lua_State* L, int idx) {
		if (lua_checkdrawable(L, idx)) {
			if (lua_isuserdata(L, idx)) {
				return dynamic_cast<LuaDrawable*>(lua_getdrawable(L, idx).ptr());
			}
			return true;
		}

		return false;
	}

	static int drawable_create(lua_State* L) {
		int nargs = lua_gettop(L);
		if (nargs == 0 || nargs > 2 || !lua_isfunction(L, 1)) {
			return luaL_error(L, "drawable_create used incorrectly: "
					" expects (function [, animation duration])");
		}
		LuaValue drawclosure(L, 1);
		float animation_duration = 0.0f;
		if (nargs == 2) {
			animation_duration = lua_tonumber(L, 2);
		}
		LuaDrawable* ldrawable = new LuaDrawable(L, drawclosure,
				animation_duration);
		lua_pushdrawable(L, Drawable(ldrawable));
		return 1;
	}

	void lua_register_drawables(lua_State* L, const LuaValue& module) {
		luawrap::install_type<Drawable, ldraw::lua_pushdrawable,
				ldraw::lua_getdrawable, ldraw::lua_checkdrawable>();
		luawrap::install_type<LuaDrawable, lua_pushluadrawable,
				lua_getluadrawable, lua_checkluadrawable>();
		module["directional_create"].bind_function(directional_create);
		module["animation_create"].bind_function(animation_create);
		module["drawable_create"].bind_function(drawable_create);
	}

}
