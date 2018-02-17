#include <SDL_opengl.h>

#include <cstdio>
#include <cmath>

#include "draw.h"
#include "ldraw_assert.h"

#include "core/gl/VideoDriverGLES20.h"
#include "STDRenderer.h"
#include "STDMaterial.h"
#include "RenderState.h"

#include "core/VideoDriver.h"
#include "RenderState.h"
#include "core/gl/VideoDriverGLES20.h"
#include "core/gl/ShaderProgramGL.h"
#include "Material.h"
#include "STDMaterial.h"
#include "core/gl/oxgl.h"
#include "core/vertex.h"


using namespace oxygine;

void ldraw::draw_rectangle(const Colour& clr, const BBoxF& bbox) {
	RenderState rs;
	rs.material = oxygine::STDMaterial::instance;
	oxygine::RectF clip = oxygine::RectF::huge();
	rs.clip = &clip;
	STDMaterial::instance->doRender(oxygine::Color(clr.r, clr.g, clr.b, clr.a), rs, {bbox.x1, bbox.y1, bbox.width(), bbox.height()}, blend_alpha);
//    STDRenderer::instance->drawBatch();
}

void ldraw::draw_rectangle_outline(const Colour& clr, const BBoxF& bbox,
		int linewidthi) {
	float linewidth = linewidthi;
	RenderState rs;
	rs.material = oxygine::STDMaterial::instance;
	oxygine::RectF clip = oxygine::RectF::huge();
	rs.clip = &clip;
	STDMaterial::instance->doRender(oxygine::Color(clr.r, clr.g, clr.b, clr.a), rs, {bbox.x1, bbox.y1, bbox.width(), linewidth}, blend_alpha);
	STDMaterial::instance->doRender(oxygine::Color(clr.r, clr.g, clr.b, clr.a), rs,
									{bbox.x1, bbox.y1 + linewidth, linewidth, std::max(0.0f,bbox.height() - linewidth * 2)}, blend_alpha);
	STDMaterial::instance->doRender(oxygine::Color(clr.r, clr.g, clr.b, clr.a), rs, {bbox.x1, bbox.y2 - linewidth, bbox.width(), linewidth}, blend_alpha);

	STDMaterial::instance->doRender(oxygine::Color(clr.r, clr.g, clr.b, clr.a), rs,
									{bbox.x2 - linewidth, bbox.y1 + linewidth, linewidth, std::max(0.0f,bbox.height() - linewidth * 2)}, blend_alpha);
//	STDRenderer::instance->drawBatch();
}

const float PI = 3.141592f;

static void gl_draw_circle(const Colour& clr, const PosF& pos, float radius,
		bool outline) {
	if (radius < .5)
		return;
	const int SEGMENTS = 16;
	float increment = 2 * PI / SEGMENTS;
	vertexPCT2 vertices[SEGMENTS];
	int vi = 0;

	vertexPCT2 vertex;
	vertex.color = oxygine::Color(clr.r, clr.g, clr.b, clr.a).rgba();
    vertex.z = 0;
	for (float a = 0; vi < SEGMENTS; a += increment, vi += 1) {
        vertex.x = (pos.x + cos(a) * radius);
        vertex.y = (pos.y + sin(a) * radius);
        vertex.z = 0;
        vertex.u = (0.5f + cos(a) * radius);
        vertex.v = (0.5f + sin(a) * radius);
        vertices[vi] = vertex;
    }

    glLineWidth(2);
    STDMaterial::instance->setCurrent(STDMaterial::instance);
    STDRenderer::instance->drawLines((void*)vertices, sizeof(vertices));
}

void ldraw::draw_circle(const Colour& clr, const PosF& pos, float radius) {
	gl_draw_circle(clr, pos, radius, false);
}

void ldraw::draw_circle_outline(const Colour& clr, const PosF& pos,
		float radius, int linewidth) {
	gl_draw_circle(clr, pos, radius, true);
}

void ldraw::draw_line(const Colour& clr, const PosF& p1, const PosF& p2,
		int linewidthi) {

    float linewidth = linewidthi;
    RenderState rs;
    rs.material = oxygine::STDMaterial::instance;
    oxygine::RectF clip = oxygine::RectF::huge();
    rs.clip = &clip;
    float len = (p2 - p1).magnitude();
    float angle = atan2(p2.y - p1.y, p2.x - p1.x);
    rs.transform.rotate(angle);
    STDMaterial::instance->doRender(oxygine::Color(clr.r, clr.g, clr.b, clr.a), rs, {p1.x, p1.y, len, linewidth}, blend_alpha);
//    STDRenderer::instance->drawBatch();
}