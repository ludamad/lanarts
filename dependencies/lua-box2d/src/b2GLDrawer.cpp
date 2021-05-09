/*
  ==============================================================================

   This file is part of the LUBYK project (http://lubyk.org)
   Copyright (c) 2007-2011 by Gaspard Bucher (http://teti.ch).

  ------------------------------------------------------------------------------

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

  ==============================================================================
*/
#include "b2GLDrawer.h"
#include <cstdio>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

void
b2GLDrawer::DrawPolygon(const b2Vec2* vertices,
                    int32 vertexCount,
                    const b2Color& color)
{
//	glColor3f(color.r, color.g, color.b);
//
//	glBegin(GL_LINE_LOOP);
//	{
//		for (int32 i = 0; i < vertexCount; ++i)
//		{
//			glVertex2f(vertices[i].x, vertices[i].y);
//		}
//	}
//	glEnd();
}

void
b2GLDrawer::DrawSolidPolygon(const b2Vec2* vertices,
                         int32 vertexCount,
                         const b2Color& color)
{
//	glEnable(GL_BLEND);
//	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//	glColor4f(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);
//
//	glBegin(GL_TRIANGLE_FAN);
//	{
//		for (int32 i = 0; i < vertexCount; ++i)
//		{
//			glVertex2f(vertices[i].x, vertices[i].y);
//		}
//	}
//	glEnd();
//
//	glDisable(GL_BLEND);
//
//	glColor4f(color.r, color.g, color.b, 1.0f);
//
//	glBegin(GL_LINE_LOOP);
//	{
//		for (int32 i = 0; i < vertexCount; ++i)
//		{
//			glVertex2f(vertices[i].x, vertices[i].y);
//		}
//	}
//	glEnd();

}

void
b2GLDrawer::DrawCircle(const b2Vec2& center,
                   float32 radius,
                   const b2Color& color)
{
//	const float32 k_segments = 16.0f;
//	const float32 k_increment = 2.0f * b2_pi / k_segments;
//
//	float32 theta = 0.0f;
//	glColor3f(color.r, color.g, color.b);
//
//	glBegin(GL_LINE_LOOP);
//	{
//		for (int32 i = 0; i < k_segments; ++i)
//		{
//			b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
//			glVertex2f(v.x, v.y);
//			theta += k_increment;
//		}
//	}
//	glEnd();

}

void
b2GLDrawer::DrawSolidCircle(const b2Vec2& center,
                        float32 radius,
                        const b2Vec2& axis,
                        const b2Color& color)
{
//	const float32 k_segments = 16.0f;
//	const float32 k_increment = 2.0f * b2_pi / k_segments;
//
//	float32 theta = 0.0f;
//
//	glEnable(GL_BLEND);
//	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//	glColor4f(0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f);
//
//	glBegin(GL_TRIANGLE_FAN);
//	{
//		for (int32 i = 0; i < k_segments; ++i)
//		{
//			b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
//			glVertex2f(v.x, v.y);
//			theta += k_increment;
//		}
//	}
//	glEnd();
//	glDisable(GL_BLEND);
//
//	theta = 0.0f;
//	glColor4f(color.r, color.g, color.b, 1.0f);
//
//	glBegin(GL_LINE_LOOP);
//	{
//		for (int32 i = 0; i < k_segments; ++i)
//		{
//			b2Vec2 v = center + radius * b2Vec2(cosf(theta), sinf(theta));
//			glVertex2f(v.x, v.y);
//			theta += k_increment;
//		}
//	}
//	glEnd();
//
//	b2Vec2 p = center + radius * axis;
//
//	glBegin(GL_LINES);
//	{
//		glVertex2f(center.x, center.y);
//		glVertex2f(p.x, p.y);
//	}
//	glEnd();

}

void
b2GLDrawer::DrawSegment(const b2Vec2& p1,
                    const b2Vec2& p2,
                    const b2Color& color)
{
//	glColor3f(color.r, color.g, color.b);
//
//	glBegin(GL_LINES);
//	{
//		glVertex2f(p1.x, p1.y);
//		glVertex2f(p2.x, p2.y);
//	}
//	glEnd();
}

void
b2GLDrawer::DrawTransform(const b2Transform& xf)
{
//	b2Vec2 p1 = xf.p, p2;
//	const float32 k_axisScale = 0.4f;
//	glBegin(GL_LINES);
//
//	glColor3f(1.0f, 0.0f, 0.0f);
//	glVertex2f(p1.x, p1.y);
//	p2 = p1 + k_axisScale * xf.q.GetXAxis();
//	glVertex2f(p2.x, p2.y);
//
//	glColor3f(0.0f, 1.0f, 0.0f);
//	glVertex2f(p1.x, p1.y);
//	p2 = p1 + k_axisScale * xf.q.GetYAxis();
//	glVertex2f(p2.x, p2.y);
//
//	glEnd();
}

