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

class LuaDrawable: public ldraw::DrawableBase {
public:
	LuaDrawable(const LuaValue& draw_closure = LuaValue(),
			float _animation_duration = 0);
	virtual ~LuaDrawable();

	virtual void draw(const DrawOptions& options, const Posf& pos) const;

	virtual float animation_duration() const {
		return _animation_duration;
	}

private:
	LuaValue draw_closure;
	float _animation_duration;
};

}
#endif
