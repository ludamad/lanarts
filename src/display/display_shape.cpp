/*
 * display_shape.cpp:
 *  Shape drawing utility functions
 */

#include <cmath>
#include <cstring>
#include <cstdio>

#undef GL_GLEXT_VERSION
#include <SDL_opengl.h>

#include "../gamestate/GameView.h"

#include "display.h"
#include "GLImage.h"

const float DEG2RAD = 3.14159 / 180;

void gl_draw_circle(float x, float y, float radius, const Colour& clr,
		bool outline) {
	if (outline)
		glBegin(GL_LINE_STRIP);
	else
		glBegin(GL_POLYGON);
	glColor4ub(clr.r, clr.g, clr.b, clr.a);
	for (int i = 0; i < 360; i++) {
		float degInRad = i * DEG2RAD;
		glVertex2f(x + cos(degInRad) * radius, y + sin(degInRad) * radius);
	}

	glEnd();
}
void gl_draw_rectangle(int x, int y, int w, int h, const Colour& clr) {
	int x2 = x + w, y2 = y + h;

	glColor4ub(clr.r, clr.g, clr.b, clr.a);
	//Draw our four points, clockwise.
	glBegin(GL_QUADS);
	glVertex2i(x, y);
	glVertex2i(x2, y);
	glVertex2i(x2, y2);
	glVertex2i(x, y2);

	glEnd();
}

void gl_draw_statbar(const BBox& bbox, int minstat, int maxstat,
		const Colour& front, const Colour& back) {
	int bar_width = (bbox.width() * minstat) / maxstat;
	gl_draw_rectangle(bbox.x1, bbox.y1, bbox.width(), bbox.height(), back);
	if (bar_width > 0) {
		gl_draw_rectangle(bbox.x1, bbox.y1, bar_width, bbox.height(), front);
	}
}
void gl_draw_rectangle_parts(int x, int y, int w, int h, int sub_parts,
		char* flags, const Colour& clr) {
	glBegin(GL_QUADS);
	int xincr = w / sub_parts, yincr = h / sub_parts;
	for (int ysub = 0; ysub < sub_parts; ysub++) {
		for (int xsub = 0; xsub < sub_parts; xsub++, flags++) {
			glColor4ub(clr.r, clr.g, clr.b, clr.a);
			if (*flags)
				continue;
			int sx = x + xincr * xsub, ex = sx + xincr;
			int sy = y + yincr * ysub, ey = sy + yincr;
			//Draw our four points, clockwise.
			glVertex2i(sx, sy);
			glVertex2i(ex, sy);
			glVertex2i(ex, ey);
			glVertex2i(sx, ey);
		}
	}
	glEnd();
}

void gl_draw_circle(const GameView& view, float x, float y, float radius,
		const Colour& colour, bool outline) {
	gl_draw_circle(x - view.x, y - view.y, radius, colour, outline);
}
void gl_draw_rectangle(const GameView& view, int x, int y, int w, int h,
		const Colour& colour) {
	gl_draw_rectangle(x - view.x, y - view.y, w, h, colour);
}

void gl_draw_rectangle_outline(int x, int y, int w, int h, const Colour& clr,
		int linewidth) {
	int x2 = x + w, y2 = y + h;

	if (linewidth != 1)
		glLineWidth(linewidth);

	glColor4ub(clr.r, clr.g, clr.b, clr.a);
	//Draw our four points, clockwise.

	glBegin(GL_LINE_STRIP);

	glVertex2i(x - linewidth, y);
	glVertex2i(x2 - linewidth, y);
	glVertex2i(x2 - linewidth, y2 - linewidth);
	glVertex2i(x, y2 - linewidth);
	glVertex2i(x, y);

	glEnd();

	if (linewidth != 1)
		glLineWidth(1);
}

void gl_draw_line(int x1, int y1, int x2, int y2, const Colour& clr,
		int linewidth) {

	if (linewidth != 1)
		glLineWidth(linewidth);

	glBegin(GL_LINES);
	glColor4ub(clr.r, clr.g, clr.b, clr.a);

	glVertex2i(x1, y1);
	glVertex2i(x2, y2);

	glEnd();

	if (linewidth != 1)
		glLineWidth(1);
}

void gl_draw_statbar(const GameView& view, const BBox& bbox, int min_stat,
		int max_stat, const Colour& front, const Colour& back) {
	gl_draw_statbar(bbox.translated(-view.x, -view.y), min_stat, max_stat,
			front, back);
}

