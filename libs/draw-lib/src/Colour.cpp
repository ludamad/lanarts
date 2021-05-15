/*
 * Colour.cpp:
 *  Defines a colour structure
 */

#include <lcommon/math_util.h>

#include "imgui.h"

#include "Colour.h"

Colour Colour::mute_colour(float val) const {
	return inverse().scale(val).inverse();
}

Colour Colour::alpha(int alpha) const {
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

Colour Colour::clamp() const {
	return Colour(squish(r, 0, 256), squish(g, 0, 256), squish(b, 0, 256),
			squish(a, 0, 256));
}

unsigned int Colour::as_rgba() const {
	ImU32 out = ((ImU32)r) << IM_COL32_R_SHIFT;
	out |= ((ImU32)g) << IM_COL32_G_SHIFT;
	out |= ((ImU32)b) << IM_COL32_B_SHIFT;
	out |= ((ImU32)a) << IM_COL32_A_SHIFT;
	return out;
}