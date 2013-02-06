/*
 * DrawableBase.cpp:
 *  A drawable object, potentially an animation.
 */

#include <lcommon/fatal_error.h>

#include "DrawableBase.h"

namespace ldraw {

SizeF DrawableBase::size() const {
	fatal_error("DrawableBase::size not implemented!\n");
	return SizeF();
}

}
