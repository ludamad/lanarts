/*
 * Colour.cpp:
 *  Defines a colour structure
 */

#include "Colour.h"

Colour Colour::mute_colour(float val) const {
	return inverse().scale(val).inverse();
}

Colour Colour::with_alpha(int alpha) const {
	return Colour(r, g, b, alpha);
}

Colour Colour::scale(float val) const {
	return Colour(r * val, g * val, b * val, a * val);
}

Colour Colour::inverse() const {
	return Colour(255 - r, 255 - g, 255 - b, 255 - a);
}

Colour Colour::mult_alpha(float alpha) const {
	return Colour(r, g, b, a * alpha);
}

Colour Colour::multiply(const Colour & o) const {
	return Colour(r * (o.r + 1) / 256, g * (o.g + 1) / 256, b * (o.b + 1) / 256,
			a * (o.a + 1) / 256);
}
