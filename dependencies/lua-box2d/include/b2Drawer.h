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
#ifndef LUBYK_INCLUDE_B2_DRAWER_H_
#define LUBYK_INCLUDE_B2_DRAWER_H_

#include "Box2D/Box2D.h"
#include "dub/dub.h"

/** @dub ignore: DrawPolygon, DrawSolidPolygon, DrawCircle, DrawSolidCircle, DrawSegment, DrawTransform
 *       push: pushobject
 */
class b2Drawer : public b2Draw, public dub::Thread
{
public:
  void DrawPolygon(const b2Vec2* vertices,
                   int32 vertexCount,
                   const b2Color& color) {
    if (!dub_pushcallback("DrawPolygon")) return;
    // <func> <self>
    lua_State *L = dub_L;

    lua_newtable(L);
    int table_idx = lua_gettop(L);
    // <func> <self> <vertices table>
    for(int i = 1; i <= vertexCount; ++i) {
      dub_pushudata(L, const_cast<b2Vec2*>(vertices + i - 1), "b2.Vec2", false);
      // <func> <self> <table> <vertex>
      lua_rawseti(L, table_idx, i);
    }
    // <func> <self> <vertices>
    lua_pushnumber(L, vertexCount);
    // <func> <self> <vertices> <vertexCount>
    dub_pushudata(L, const_cast<b2Color*>(&color), "b2.Color", false);
    // <func> <self> <vertices> <vertexCount> <color>
    dub_call(4, 0);
  }

  void DrawSolidPolygon(const b2Vec2* vertices,
                        int32 vertexCount,
                        const b2Color& color) {
    if (!dub_pushcallback("DrawSolidPolygon")) return;
    // <func> <self>
    lua_State *L = dub_L;
    lua_newtable(L);
    int table_idx = lua_gettop(L);
    // <func> <self> <vertices table>
    for(int i = 1; i <= vertexCount; ++i) {
      dub_pushudata(L, const_cast<b2Vec2*>(vertices + i - 1), "b2.Vec2", false);
      // <func> <self> <table> <vertex>
      lua_rawseti(L, table_idx, i);
    }
    // <func> <self> <vertices>

    // <func> <self> <vertices>
    lua_pushnumber(L, vertexCount);
    // <func> <self> <vertices> <vertexCount>
    dub_pushudata(L, const_cast<b2Color*>(&color), "b2.Color", false);
    // <func> <self> <vertices> <vertexCount> <color>
    dub_call(4, 0);
  }

  void DrawCircle(const b2Vec2& center,
                  float32 radius,
                  const b2Color& color) {
    if (!dub_pushcallback("DrawCircle")) return;
    // <func> <self>
    lua_State *L = dub_L;

    dub_pushudata(L, const_cast<b2Vec2*>(&center), "b2.Vec2", false);
    // <func> <self> <center>
    lua_pushnumber(L, radius);
    // <func> <self> <center> <radius>
    dub_pushudata(L, const_cast<b2Color*>(&color), "b2.Color", false);
    // <func> <self> <center> <radius> <color>
    dub_call(4, 0);
  }

  void DrawSolidCircle(const b2Vec2& center,
                       float32 radius,
                       const b2Vec2& axis,
                       const b2Color& color) {
    if (!dub_pushcallback("DrawSolidCircle")) return;
    // <func> <self>
    lua_State *L = dub_L;

    dub_pushudata(L, const_cast<b2Vec2*>(&center), "b2.Vec2", false);
    // <func> <self> <center>
    lua_pushnumber(L, radius);
    // <func> <self> <center> <radius>
    dub_pushudata(L, const_cast<b2Vec2*>(&axis), "b2.Vec2", false);
    // <func> <self> <center> <radius> <axis>
    dub_pushudata(L, const_cast<b2Color*>(&color), "b2.Color", false);
    // <func> <self> <center> <radius> <axis> <color>
    dub_call(5, 0);
  }

  void DrawSegment(const b2Vec2& p1,
                   const b2Vec2& p2,
                   const b2Color& color) {
    if (!dub_pushcallback("DrawSegment")) return;
    // <func> <self>
    lua_State *L = dub_L;

    dub_pushudata(L, const_cast<b2Vec2*>(&p1), "b2.Vec2", false);
    // <func> <self> <p1>
    dub_pushudata(L, const_cast<b2Vec2*>(&p2), "b2.Vec2", false);
    // <func> <self> <p1> <p2>
    dub_pushudata(L, const_cast<b2Color*>(&color), "b2.Color", false);
    // <func> <self> <p1> <p2> <color>
    dub_call(4, 0);
  }

  void DrawTransform(const b2Transform& xf) {
    if (!dub_pushcallback("DrawTransform")) return;
    // <func> <self>
    lua_State *L = dub_L;

    dub_pushudata(L, const_cast<b2Transform*>(&xf), "b2.Transform", false);
    // <func> <self> <xf>
    dub_call(2, 0);
  }
};

#endif // LUBYK_INCLUDE_B2_DRAWER_H_

