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
    STDRenderer::instance->drawBatch();
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
	STDRenderer::instance->drawBatch();
}

const float PI = 3.141592f;
const float DEG2RAD = PI / 180.0f;

static void drawPrimitives(bool drawTriangles, bool drawLines, int count, const Colour& color, GLfloat* vertices);

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
    STDRenderer::instance->drawBatch();

}

///// Draw a circle.
//void DrawCircle(const b2Vec2& center, float32 aRadius, const b2Color& color)
//{
//    createCircleVertices(center, aRadius);
//    drawPrimitives(false, true, CIRCLE_SEGMENTS, color);
//}
//
///// Draw a solid circle.
//void DrawSolidCircle(const b2Vec2& center, float32 aRadius, const b2Vec2& aAxis,
//                                const b2Color& color)
//{
//    createCircleVertices(center, aRadius);
//    drawPrimitives(true, true, CIRCLE_SEGMENTS, color);
//    // Draw the axis line
//    DrawSegment(center, center + aRadius * aAxis, color);
//}
//
///// Draw a line segment.
//void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
//{
//    mVertices[0].x = p1.x * _worldScale;
//    mVertices[0].y = p1.y * _worldScale;
//    mVertices[1].x = p2.x * _worldScale;
//    mVertices[1].y = p2.y * _worldScale;
//    drawPrimitives(false, true, 2, color);
//}
//
///// Draw a transform. Choose your own length scale.
///// @param xf a transform.
//void DrawTransform(const b2Transform& xf)
//{
//    b2Vec2 p1 = xf.p, p2;
//    const float32 k_axisScale = 0.4f;
//
//    p2 = p1 + k_axisScale * xf.q.GetXAxis();
//    DrawSegment(p1, p2, b2Color(1, 0, 0));
//
//    p2 = p1 + k_axisScale * xf.q.GetYAxis();
//    DrawSegment(p1, p2, b2Color(0, 1, 0));
//}
//
//void createCircleVertices(const b2Vec2& center, float32 aRadius)
//{
//    int vertexCount = 16;
//    const float32 k_increment = 2.0f * b2_pi / CIRCLE_SEGMENTS;
//    float32 theta = 0.0f;
//
//    for (int32 i = 0; i < CIRCLE_SEGMENTS; ++i)
//    {
//        b2Vec2 v = center + aRadius * b2Vec2(scalar::cos(theta), scalar::sin(theta));
//        mVertices[i].x = _worldScale * v.x;
//        mVertices[i].y = _worldScale * v.y;
//        theta += k_increment;
//    }
//}
//
//void createPolygonVertices(const b2Vec2* vertices, int32 vertexCount)
//{
//    if (vertexCount > MAX_VERTICES)
//    {
//        log::warning("need more vertices");
//        return;
//    }
//
//    // convert vertices to screen resolution
//    for (int i = 0; i < vertexCount; i++)
//    {
//        mVertices[i].x = _worldScale * vertices[i].x;
//        mVertices[i].y = _worldScale * vertices[i].y;
//    }
//}


static ShaderProgramGL* _program;

//------------------------------------------------------------------------
static void drawPrimitives(bool drawTriangles, bool drawLines, int count, const Colour& color, GLfloat* vertices) {

	if (!_program) {
		const char *vertexShaderData = "\
									uniform mediump mat4 projection;\
									attribute vec2 a_position;\
									void main() {\
									vec4 position = vec4(a_position, 0.0, 1.0);\
									gl_Position = projection * position;\
									}\
									";

		const char *fragmentShaderData = "\
									  uniform mediump vec4 color;\
									  void main() { \
									  gl_FragColor = color; \
									  } \
									  ";


		int vs = ShaderProgramGL::createShader(GL_VERTEX_SHADER, vertexShaderData, 0, 0);
		int fs = ShaderProgramGL::createShader(GL_FRAGMENT_SHADER, fragmentShaderData, 0, 0);

		int pr = ShaderProgramGL::createProgram(vs, fs,
												(VertexDeclarationGL *) IVideoDriver::instance->getVertexDeclaration(
														VERTEX_POSITION));

		_program = new ShaderProgramGL(pr);
	}
    auto* prev_program = IVideoDriver::instance->getShaderProgram();
	IVideoDriver::instance->setShaderProgram(_program);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);

    if (drawTriangles)
    {
        Vector4 c(color.r, color.g, color.b, 0.5f);
        IVideoDriver::instance->setUniform("color", &c, 1);
        glDrawArrays(GL_TRIANGLE_FAN, 0, count / 2);
    }

    if (drawLines)
    {
        Vector4 c(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
        IVideoDriver::instance->setUniform("color", &c, 1);
        glDrawArrays(GL_LINE_LOOP, 0, count / 2);
    }

	glDisableVertexAttribArray(0);

    IVideoDriver::instance->setShaderProgram(prev_program);
}
