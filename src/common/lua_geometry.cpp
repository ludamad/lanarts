/*
 * lua_geometry.cpp:
 *  Lua bindings for geometry structures
 */

#include "lua_geometry.h"
#include "lua_numarray.h"

template<>
void SLB::push<Pos>(lua_State *L, Pos p) {
	lua_push_numarray(L, (const int*)&p, sizeof(Pos) / sizeof(int));
}

template<>
Pos SLB::get<Pos>(lua_State *L, int pos) {
	Pos p;
	lua_to_numarray(L, pos, (int*)&p, sizeof(Pos) / sizeof(int));
	return p;
}

template<>
void SLB::push<Posf>(lua_State *L, Posf p) {
	lua_push_numarray(L, (const float*)&p, sizeof(Posf) / sizeof(int));
}

template<>
Posf SLB::get<Posf>(lua_State *L, int pos) {
	Posf p;
	lua_to_numarray(L, pos, (float*)&p, sizeof(Posf) / sizeof(int));
	return p;
}
