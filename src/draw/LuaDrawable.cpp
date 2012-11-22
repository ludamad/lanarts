/*
 * LuaDrawable.cpp:
 *  Represents a drawable that encapsulates a lua closure and a draw duration.
 *  A draw duration of 0 (or nil in lua) signifies a non-animated drawable.
 */

#include <SLB/LuaCall.hpp>
#include <common/lua/lua_geometry.h>

#include "lua/lua_drawoptions.h"

#include "LuaDrawable.h"

namespace ldraw {

LuaDrawable::LuaDrawable(lua_State *L, const LuaValue & draw_closure,
		float _animation_duration) :
		L(L), draw_closure(draw_closure), _animation_duration(
				_animation_duration) {
}

LuaDrawable::~LuaDrawable() {
}

void LuaDrawable::draw(const DrawOptions & options, const Posf & pos) const {
	draw_closure.push(L);
	SLB::LuaCall<void(const DrawOptions&, const Posf&)> call(L, -1);
	call(options, pos);
}

bool LuaDrawable::operator ==(const LuaDrawable & o) const {
	if (L != o.L || animation_duration() != o.animation_duration()) {
		return false;
	}
	draw_closure.push(L);
	o.draw_closure.push(L);
	bool eq = lua_equal(L, -1, -2);
	lua_pop(L, 2);
	return eq;
}

bool LuaDrawable::operator !=(const LuaDrawable & o) const {
	return !(*this == o);
}

}
