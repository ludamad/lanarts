#include <SDL2/SDL_opengl.h>

#include <cstdio>
#include <cmath>

#include "draw.h"
#include "ldraw_assert.h"

void ldraw::draw_rectangle(const Colour& clr, const BBoxF& bbox) {
	glColor4ub(clr.r, clr.g, clr.b, clr.a);
	//Draw our four points, clockwise.
	glBegin(GL_QUADS);
	glVertex2i(bbox.x1, bbox.y1);
	glVertex2i(bbox.x2, bbox.y1);
	glVertex2i(bbox.x2, bbox.y2);
	glVertex2i(bbox.x1, bbox.y2);

	glEnd();
}

void ldraw::draw_rectangle_outline(const Colour& clr, const BBoxF& bbox,
		int linewidth) {
	if (linewidth != 1)
		glLineWidth(linewidth);

	glColor4ub(clr.r, clr.g, clr.b, clr.a);
	//Draw our four points, clockwise.

	glBegin(GL_LINE_STRIP);

	glVertex2i(bbox.x1 - linewidth, bbox.y1);
	glVertex2i(bbox.x2 - linewidth, bbox.y1);
	glVertex2i(bbox.x2 - linewidth, bbox.y2 - linewidth);
	glVertex2i(bbox.x1, bbox.y2 - linewidth);
	glVertex2i(bbox.x1, bbox.y1);

	glEnd();

	if (linewidth != 1)
		glLineWidth(1);
}
const float PI = 3.141592f;
const float DEG2RAD = PI / 180.0f;

static void gl_draw_circle(const Colour& clr, const PosF& pos, float radius,
		bool outline) {
	if (radius < .5)
		return;
	if (outline)
		glBegin(GL_LINE_STRIP);
	else
		glBegin(GL_POLYGON);
	glColor4ub(clr.r, clr.g, clr.b, clr.a);

	float circum = radius * PI;
	float iter = PI * 2 / circum;
	for (float a = 0; a < PI * 2 + iter; a += iter) {
		glVertex2f(pos.x + cos(a) * radius, pos.y + sin(a) * radius);
	}

	glEnd();
}

void ldraw::draw_circle(const Colour& clr, const PosF& pos, float radius) {
	gl_draw_circle(clr, pos, radius, false);
}

void ldraw::draw_circle_outline(const Colour& clr, const PosF& pos,
		float radius, int linewidth) {
	if (linewidth != 1)
		glLineWidth(linewidth);
	gl_draw_circle(clr, pos, radius, true);
	if (linewidth != 1)
		glLineWidth(1);
}

void ldraw::draw_line(const Colour& clr, const PosF& p1, const PosF& p2,
		int linewidth) {

	if (linewidth != 1)
		glLineWidth(linewidth);

	glBegin(GL_LINES);
	glColor4ub(clr.r, clr.g, clr.b, clr.a);

	glVertex2i(p1.x, p1.y);
	glVertex2i(p2.x, p2.y);

	glEnd();

	if (linewidth != 1)
		glLineWidth(1);
}
