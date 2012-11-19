/*
 * lua_geometry.h:
 *  Lua bindings for geometry structures
 */

#ifndef LUA_GEOMETRY_H_
#define LUA_GEOMETRY_H_

#include "../geometry.h"

#include "slb_numvectortype.h"

SLB_NUMERIC_VECTOR(Pos, int);
SLB_NUMERIC_VECTOR(Posf, float);
SLB_NUMERIC_VECTOR(Dim, int);
SLB_NUMERIC_VECTOR(DimF, float);
SLB_NUMERIC_VECTOR(BBox, int);
SLB_NUMERIC_VECTOR(BBoxF, float);


#endif /* LUA_GEOMETRY_H_ */
