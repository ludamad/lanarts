/*
 * Drawable.cpp:
 *  Encapsulates a pointer to a drawable object
 */

#include "DrawOptions.h"
#include "DrawableBase.h"
#include "Drawable.h"

namespace ldraw {

Drawable::Drawable() {
}

Drawable::Drawable(const smartptr<DrawableBase>& drawable) :
		ref(drawable) {
}

Drawable::~Drawable() {
}

void Drawable::draw(const DrawOptions & options, const PosF & pos) const {
	return ref->draw(options, pos);
}

void Drawable::draw(const PosF & pos) const {
	ref->draw(pos);
}

float Drawable::animation_duration() const {
	return ref->animation_duration();
}

Drawable& Drawable::operator =(const smartptr<DrawableBase> & drawable) {
	ref = drawable;
	return *this;
}

bool Drawable::is_animated() const {
	return ref->is_animated();
}

SizeF Drawable::size() const {
	return ref->size();
}

}
