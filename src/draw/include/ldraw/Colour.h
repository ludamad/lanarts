/* Colour.h:
 *  Defines a colour structure
 */

#ifndef COLOUR_H_
#define COLOUR_H_

struct Colour {
	int r, g, b, a;
	Colour(int r = 255, int g = 255, int b = 255, int a = 255) :
			r(r), g(g), b(b), a(a) {
	}
	bool operator==(const Colour& col) const {
		return r == col.r && g == col.g && b == col.b && a == col.a;
	}
	bool operator!=(const Colour& col) const {
		return !(*this == col);
	}
	Colour alpha(int alpha) const;
	Colour scale(float val) const;

	Colour inverse() const;
	Colour mute_colour(float val) const;
	Colour mult_alpha(float alpha) const;
	Colour multiply(const Colour& o) const;
	Colour clamp() const;
};

#endif /* COLOUR_H_ */
