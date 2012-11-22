/*
 * LuaDrawable.h:
 *  Represents a drawable that encapsulates a lua closure and a draw duration.
 *  A draw duration of 0 (or nil in lua) signifies a non-animated drawable.
 */

#ifndef LUADRAWABLE_H_
#define LUADRAWABLE_H_

#include <common/lua/LuaValue.h>
#include "DrawableBase.h"

struct lua_State;

namespace ldraw {

class LuaDrawable: public DrawableBase {
public:
	LuaDrawable(lua_State* L, const LuaValue& draw_closure = LuaValue(),
			float _animation_duration = 0);
	~LuaDrawable();

	void draw(const DrawOptions& options, const Posf& pos) const;

	float animation_duration() const {
		return _animation_duration;
	}

	bool operator==(const LuaDrawable& o) const;
	bool operator!=(const LuaDrawable& o) const;


private:
	lua_State* L;
	LuaValue draw_closure;
	float _animation_duration;
};

// Implemented in lua_drawable.cpp
void lua_pushluadrawable(lua_State* L, const LuaDrawable& image);
LuaDrawable lua_getluadrawable(lua_State* L, int idx);
bool lua_checkluadrawable(lua_State* L, int idx);

}
#endif