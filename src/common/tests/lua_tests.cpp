extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include "../unittest.h"

#include "../lua_geometry.h"

#include <SLB/Manager.hpp>
#include <SLB/Script.hpp>

static void lua_point_pushget_test() {
	lua_State* L = lua_open();
	/*Test Pos*/
	{
		Pos toLua(1, 2);

		SLB::push<Pos>(L, toLua);
		Pos fromLua = SLB::get<Pos>(L, -1);

		unit_test_assert("Pos get/set mismatch", toLua == fromLua);
	}
	/*Test PosF*/
	{
		Posf toLua(0.5, 1.5);

		SLB::push<Posf>(L, toLua);
		Posf fromLua = SLB::get<Posf>(L, -1);

		unit_test_assert("PosF get/set mismatch", toLua == fromLua);
	}

	lua_close(L);
}

static void pos_func(const Pos& p) {
	UNIT_TEST_ASSERT(p == Pos(1,2));
}

static void lua_point_bind_test() {
	lua_State* L = lua_open();

	SLB::Manager m;
	SLB::Script script(&m);

	m.set("pos_func", SLB::FuncCall::create(pos_func));
	script.doString("SLB.using(SLB)\n"
			"pos_func([1,2])\n");

	lua_close(L);
}

void lua_binding_tests() {
	UNIT_TEST(lua_point_pushget_test);
	UNIT_TEST(lua_point_bind_test);
}
