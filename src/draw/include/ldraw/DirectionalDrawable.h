/*
 * DirectionalDrawable.h:
 *  Represents a series of images that are drawn for various directions
 */

#ifndef LDRAW_DIRECTIONALDRAWABLE_H_
#define LDRAW_DIRECTIONALDRAWABLE_H_

#include <vector>
#include "Drawable.h"
#include "DrawableBase.h"

namespace ldraw {

class Image;

class DirectionalDrawable: public DrawableBase {
public:
	using DrawableBase::draw;

	DirectionalDrawable(const std::vector<Drawable>& directions,
			float angle_offset = 0.0f);

	template<typename T>
	DirectionalDrawable(const std::vector<T>& directions, float angle_offset =
			0.0f) :
			_angle_offset(angle_offset), _animation_duration(0.0f), _duration_was_set(
					0) {
		for (int i = 0; i < directions.size(); i++) {
			add(Drawable(new T(directions[i])));
		}
	}

	DirectionalDrawable(float angle_offset = 0.0f);

	virtual ~DirectionalDrawable();

	virtual void draw(const DrawOptions& options, const Posf& pos) const;

	void add(const Drawable& direction);

	virtual float animation_duration() const {
		return _animation_duration;
	}

	virtual SizeF size() const;

	void set_animation_duration(float duration) {
		_animation_duration = duration;
		_duration_was_set = true;
	}


private:
	std::vector<Drawable> directions;
	float _angle_offset;

	// Takes on value of first. Can be set manually, too.
	float _animation_duration;
	bool _duration_was_set;
};

}
#endif /* LDRAW_DIRECTIONALDRAWABLE_H_ */
