/*
 * DrawableBase.h:
 *  A drawable object, potentially an animation.
 */

#ifndef LDRAW_DRAWABLEBASE_H_
#define LDRAW_DRAWABLEBASE_H_

#include <lcommon/geometry.h>
#include "DrawOptions.h"

struct lua_State;

namespace ldraw {

/*abstract*/class DrawableBase {
public:
	virtual ~DrawableBase() {
	}

	virtual void draw(const DrawOptions& options, const PosF& pos) const = 0;

	/* Duration of an animation, 0 means not animated */
	virtual float animation_duration() const {
		return 0; // default to not animated
	}

	bool is_animated() const {
		return animation_duration() != 0;
	}

	void draw(const PosF& pos) const {
		draw(DrawOptions(), pos);
	}

	virtual SizeF size() const;

	/* Pushes metatable to be used with a userdata object, which encapsulates a Drawable.
	 * Implemented in LuaDrawable.cpp
	 */
	virtual void push_metatable(lua_State* L) const;
};

}

#endif /* LDRAW_DRAWABLEBASE_H_ */
