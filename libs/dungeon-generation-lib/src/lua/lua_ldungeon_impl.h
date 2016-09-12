/*
 * lua_ldungeon_impl.h:
 *  Internally shared binding functions
 */

#ifndef LUA_LDUNGEON_IMPL_H_
#define LUA_LDUNGEON_IMPL_H_

#include "Map.h"
#include "map_fill.h"

struct lua_State;
class LuaValue;
class MTwist;

namespace ldungeon_gen {
	Selector lua_selector_get(LuaField args);
	Selector lua_selector_optional_get(LuaField args);

	/* Parses an 'operators' from a table. All fields are optional. */
	Operator lua_operator_get(LuaField args);
	Operator lua_operator_optional_get(LuaField args);

	Square lua_square_get(LuaStackValue args);
	void lua_square_push(lua_State* L, Square square);

	ConditionalOperator lua_conditional_operator_get(LuaField args);
	ConditionalOperator lua_conditional_operator_optional_get(LuaField args);
	void lua_register_bsp(const LuaValue& submodule);

	int oper_aux(lua_State* L);
	AreaOperatorPtr area_operator_get(LuaField val);

	void register_libxmi_bindings(LuaValue submodule);
}

#endif /* LUA_LDUNGEON_IMPL_H_ */
