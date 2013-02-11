/*
 * DirectionalDrawable.cpp:
 *  Represents a series of images that are drawn for various directions
 */

#include <algorithm>
#include <cmath>
#include <lcommon/math_util.h>

#include "DirectionalDrawable.h"
#include "Image.h"

namespace ldraw {

DirectionalDrawable::DirectionalDrawable(
		const std::vector<Drawable> & directions, float angle_offset) :
		directions(directions), _angle_offset(angle_offset), _animation_duration(
				0), _duration_was_set(false) {
	if (!directions.empty()) {
		int anim = directions[0].animation_duration();
		set_animation_duration(anim);
	}
}

DirectionalDrawable::DirectionalDrawable(float angle_offset) :
		_angle_offset(angle_offset), _animation_duration(0), _duration_was_set(
				false) {
	if (!directions.empty()) {
		int anim = directions[0].animation_duration();
		set_animation_duration(anim);
	}
}

DirectionalDrawable::~DirectionalDrawable() {
}

void DirectionalDrawable::draw(const DrawOptions& options,
		const Posf& pos) const {
	if (directions.empty()) {
		return;
	}

	float fidx = (options.draw_angle + _angle_offset) / FLOAT_PI / 2;
	int nframes = directions.size();

	int idx = round2int(fidx * nframes) % nframes;

	if (idx < 0) { // % can leave negative values
		idx += nframes;
	}

	directions[idx].draw(DrawOptions(options).angle(0), pos);
}

void DirectionalDrawable::add(const Drawable & direction) {
	if (_duration_was_set) {
		set_animation_duration(direction.animation_duration());
	}
	directions.push_back(direction);
}

SizeF DirectionalDrawable::size() const {
	return directions.at(0).size();
}

}
