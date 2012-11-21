/*
 * lua_geometry.h:
 *  Lua bindings for geometry structures
 */

#ifndef LUA_GEOMETRY_H_
#define LUA_GEOMETRY_H_

#include "../geometry.h"

#include "luacpp_wrap_numeric.h"

LUACPP_WRAP_AS_NUMARRAY(Pos, int);
LUACPP_WRAP_AS_NUMARRAY(Posf, float);
LUACPP_WRAP_AS_NUMARRAY(Dim, int);
LUACPP_WRAP_AS_NUMARRAY(DimF, float);
LUACPP_WRAP_AS_NUMARRAY(BBox, int);
LUACPP_WRAP_AS_NUMARRAY(BBoxF, float);


#endif /* LUA_GEOMETRY_H_ */
