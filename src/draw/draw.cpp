#include <SDL_opengl.h>

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

const float PI = 3.141592f;
const float DEG2RAD = PI / 180.0f;

void ldraw::draw_circle(const Colour& clr, const Posf& pos, float radius,
		bool outline) {
	if (radius < .5)
		return;
	if (outline)
		glBegin(GL_LINE_STRIP);
	else
		glBegin(GL_POLYGON);
	glColor4ub(clr.r, clr.g, clr.b, clr.a);
	float circum = radius * PI;
	for (float a = 0; a < PI * 2; a += PI * 2 / circum) {
		glVertex2f(pos.x + cos(a) * radius, pos.y + sin(a) * radius);
	}
	glEnd();
}

