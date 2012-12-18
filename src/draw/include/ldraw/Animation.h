/*
 * Animation.h:
 *  Represents a string of frames creating an animation
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include <vector>
#include "Drawable.h"
#include "DrawableBase.h"

namespace ldraw {

class Animation: public ldraw::DrawableBase {
public:
	using DrawableBase::draw;

	Animation(const std::vector<Drawable>& frames,
			float animation_speed = 1.0f);
	Animation(float animation_speed = 1.0f);

	template<typename T>
	Animation(const std::vector<T>& frames, float animation_speed = 1.0f) :
			_animation_speed(animation_speed) {
		for (int i = 0; i < frames.size(); i++) {
			add(Drawable(new T(frames[i])));
		}
		_animation_duration = frames.size() / animation_speed;
	}

	virtual ~Animation();

	virtual void draw(const DrawOptions& options, const Posf& pos) const;

	void add(const Drawable& frame);

	virtual float animation_duration() const {
		return _animation_duration;
	}

	virtual DimF size() const;

	void set_animation_duration(float duration) {
		_animation_duration = duration;
	}
private:
	std::vector<Drawable> frames;

	// Assume frame-length, can be overridden manually.
	// Takes on value of first. Can be set manually, too.
	float _animation_duration;
	// Speed at which to play frames
	float _animation_speed;
};

} /* namespace ldraw */
#endif /* ANIMATION_H_ */
