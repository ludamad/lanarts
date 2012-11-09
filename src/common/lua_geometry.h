/*
 * lua_geometry.h:
 *  Lua bindings for geometry structures
 */

#ifndef LUA_GEOMETRY_H_
#define LUA_GEOMETRY_H_

#include <SLB/PushGet.hpp>

#include "geometry.h"

namespace SLB {
template<>
void push<Pos>(lua_State *L, Pos p);

template<>
Pos get<Pos>(lua_State *L, int pos);

template<>
void push<Posf>(lua_State *L, Posf p);

template<>
Posf get<Posf>(lua_State *L, int pos);
}

#endif /* LUA_GEOMETRY_H_ */
