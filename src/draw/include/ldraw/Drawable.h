/*
 * Drawable.h:
 *  Encapsulates a pointer to a drawable object
 */

#ifndef LDRAW_DRAWABLE_H_
#define LDRAW_DRAWABLE_H_

#include <lcommon/smartptr.h>
#include <lcommon/geometry.h>

struct Posf;
struct lua_State;

namespace ldraw {

struct DrawOptions;
class DrawableBase;

class Drawable {
public:
	Drawable(const smartptr<DrawableBase>& drawable);
	Drawable& operator=(const smartptr<DrawableBase>& drawable);
	Drawable();
	~Drawable();

	void draw(const DrawOptions& options, const Posf& pos) const;
	void draw(const Posf& pos) const;

	/* Duration of an animation, 0 means not animated */
	float animation_duration() const;
	bool is_animated() const;

	smartptr<DrawableBase> get_ref() const {
		return ref;
	}
	DrawableBase* ptr() const {
		return ref.get();
	}

	SizeF size() const;

private:
	smartptr<DrawableBase> ref;
};

// Implemented in lua_drawable.cpp
// Pushes with a different metatable based on the concrete type
void lua_pushdrawable(lua_State* L, const Drawable& drawable);
Drawable lua_getdrawable(lua_State* L, int idx);
bool lua_checkdrawable(lua_State* L, int idx);

}

#endif /* LDRAW_DRAWABLE_H_ */
