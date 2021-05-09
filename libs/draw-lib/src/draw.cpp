#include <SDL_opengl.h>

#include <cstdio>
#include <cmath>

#include "draw.h"
#include "ldraw_assert.h"
#include "imgui.h"

void ldraw::draw_rectangle(const Colour& clr, const BBoxF& bbox) {
	ImGui::GetForegroundDrawList()->AddQuadFilled(
		ImVec2(bbox.x1, bbox.y1),
		ImVec2(bbox.x2, bbox.y1),
		ImVec2(bbox.x2, bbox.y2),
		ImVec2(bbox.x1, bbox.y2),
		clr.as_rgba()
	);

//	glColor4ub(clr.r, clr.g, clr.b, clr.a);
//	//Draw our four points, clockwise.
//	glBegin(GL_QUADS);
//	glVertex2i(;
//	glVertex2i();
//	glVertex2i();
//	glVertex2i();
//
//	glEnd();
}

void ldraw::draw_rectangle_outline(const Colour& clr, const BBoxF& bbox,
		int linewidth) {

    ImGui::GetForegroundDrawList()->AddQuad(
            ImVec2(bbox.x1, bbox.y1),
            ImVec2(bbox.x2, bbox.y1),
            ImVec2(bbox.x2, bbox.y2),
            ImVec2(bbox.x1, bbox.y2),
            clr.as_rgba(),
            linewidth
    );

//    if (linewidth != 1)
//		glLineWidth(linewidth);
//
//	glColor4ub(clr.r, clr.g, clr.b, clr.a);
//	//Draw our four points, clockwise.
//
//	glBegin(GL_LINE_STRIP);
//
//	glVertex2i(bbox.x1 - linewidth, bbox.y1);
//	glVertex2i(bbox.x2 - linewidth, bbox.y1);
//	glVertex2i(bbox.x2 - linewidth, bbox.y2 - linewidth);
//	glVertex2i(bbox.x1, bbox.y2 - linewidth);
//	glVertex2i(bbox.x1, bbox.y1);
//
//	glEnd();
//
//	if (linewidth != 1)
//		glLineWidth(1);
}
const float PI = 3.141592f;
const float DEG2RAD = PI / 180.0f;

void ldraw::draw_circle(const Colour& clr, const PosF& pos, float radius) {
    if (radius < .5) {
        return;
    }
    ImGui::GetForegroundDrawList()->AddCircleFilled(
        ImVec2(pos.x, pos.y),
        radius,
        clr.as_rgba()
    );
}

void ldraw::draw_circle_outline(const Colour& clr, const PosF& pos,
		float radius, int linewidth) {
    if (radius < .5) {
        return;
    }
    ImGui::GetForegroundDrawList()->AddCircle(
        ImVec2(pos.x, pos.y),
        radius,
        clr.as_rgba(),
        0,
        linewidth
    );
}

void ldraw::draw_line(const Colour& clr, const PosF& p1, const PosF& p2,
		int linewidth) {

    ImGui::GetForegroundDrawList()->AddLine(
        ImVec2(p1.x, p1.y),
        ImVec2(p2.x, p2.y),
        clr.as_rgba(),
        linewidth
    );
//	if (linewidth != 1)
//		glLineWidth(linewidth);
//
//	glBegin(GL_LINES);
//	glColor4ub(clr.r, clr.g, clr.b, clr.a);
//
//	glVertex2i(p1.x, p1.y);
//	glVertex2i(p2.x, p2.y);
//
//	glEnd();
//
//	if (linewidth != 1)
//		glLineWidth(1);
}
