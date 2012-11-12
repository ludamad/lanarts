/*
 * lua_geometry.h:
 *  Lua bindings for geometry structures
 */

#ifndef LUA_GEOMETRY_H_
#define LUA_GEOMETRY_H_

#include "../geometry.h"

#include "slb_valuetype.h"

SLB_WRAP_VALUE_TYPE_AS_NUMARR(Pos, int);
SLB_WRAP_VALUE_TYPE_AS_NUMARR(Posf, float);
SLB_WRAP_VALUE_TYPE_AS_NUMARR(Dim, int);
SLB_WRAP_VALUE_TYPE_AS_NUMARR(DimF, float);
SLB_WRAP_VALUE_TYPE_AS_NUMARR(BBox, int);
SLB_WRAP_VALUE_TYPE_AS_NUMARR(BBoxF, float);

#endif /* LUA_GEOMETRY_H_ */
