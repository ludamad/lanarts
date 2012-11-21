/*
 * lua_geometry.h:
 *  Lua bindings for geometry structures
 */

#include "lua_geometry.h"

LUACPP_WRAP_AS_NUMARRAY_IMPL(Pos, int);
LUACPP_WRAP_AS_NUMARRAY_IMPL(Posf, float);
LUACPP_WRAP_AS_NUMARRAY_IMPL(Dim, int);
LUACPP_WRAP_AS_NUMARRAY_IMPL(DimF, float);
LUACPP_WRAP_AS_NUMARRAY_IMPL(BBox, int);
LUACPP_WRAP_AS_NUMARRAY_IMPL(BBoxF, float);
