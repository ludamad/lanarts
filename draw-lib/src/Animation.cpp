/*
 * Animation.cpp:
 *  Represents a string of frames creating an animation
 */

#include <lcommon/math_util.h>

#include "ldraw_assert.h"
#include "Animation.h"

namespace ldraw {

Animation::Animation(const std::vector<Drawable> & frames,
		float animation_speed) :
		frames(frames), _animation_speed(animation_speed) {
	LDRAW_ASSERT(animation_speed != 0.0f);
	_animation_duration = frames.size() / _animation_speed;
}

Animation::Animation(float animation_speed) :
		_animation_duration(0), _animation_speed(animation_speed) {
}

Animation::~Animation() {
}

void Animation::draw(const DrawOptions& options, const PosF& pos) const {
	if (frames.empty()) {
		return;
	}

	float fidx = options.draw_frame * _animation_speed;
	int nframes = frames.size();

	int idx = iround(fidx) % nframes;

	if (idx < 0) { // % can leave negative values
		idx += nframes;
	}

	frames[idx].draw(DrawOptions(options).frame(0), pos);
}

void Animation::add(const Drawable & frame) {
	frames.push_back(frame);
}

SizeF Animation::size() const {
	return frames.at(0).size();
}

}
