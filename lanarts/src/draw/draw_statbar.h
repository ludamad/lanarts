/*
 * draw_statbar.h:
 *  Function to draw a simple stat bar, consisting of two rectangles.
 */

#ifndef DRAW_STATBAR_H_
#define DRAW_STATBAR_H_

struct BBoxF;
struct Colour;

void draw_statbar(const BBoxF& bbox, float fill, const Colour& front,
		const Colour& back);
void draw_statbar(const BBoxF& bbox, float fill);

#endif /* DRAW_STATBAR_H_ */
