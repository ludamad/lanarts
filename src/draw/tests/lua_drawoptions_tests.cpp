/*
 * lua_drawoptions_tests.cpp:
 *  Test drawoptions bindings in lua
 */

#include <SLB/Manager.hpp>
#include <SLB/FuncCall.hpp>

#include <common/lua/lua_unittest.h>
#include <common/lua/lua_geometry.h>

#include "../lua/lua_colour.h"
#include "../lua/lua_drawoptions.h"

static void drawoptions_func_defaults(const ldraw::DrawOptions& options) {
	ldraw::DrawOptions defaults;
	UNIT_TEST_ASSERT(options.draw_colour == defaults.draw_colour);
	UNIT_TEST_ASSERT(options.draw_origin == defaults.draw_origin);
	UNIT_TEST_ASSERT(options.draw_region == defaults.draw_region);
}
static void drawoptions_func_difforigin(const ldraw::DrawOptions& options) {
	ldraw::DrawOptions defaults;
	UNIT_TEST_ASSERT(options.draw_colour == defaults.draw_colour);
	UNIT_TEST_ASSERT(options.draw_origin == ldraw::CENTER);
	UNIT_TEST_ASSERT(options.draw_region == defaults.draw_region);
}

static void drawoptions_func_diffall(const ldraw::DrawOptions& options) {
	UNIT_TEST_ASSERT(options.draw_colour == Colour(1,2,3,4));
	UNIT_TEST_ASSERT(options.draw_origin == ldraw::CENTER);
	UNIT_TEST_ASSERT(options.draw_region == BBoxF(1,2,3,4));
}

static void lua_drawoptions_bind_test() {
	lua_State* L = lua_open();

	SLB::Manager m;
	m.registerSLB(L);

	lua_register_draworigin_constants(L, m.getGlobals());
	m.set("drawoptions_func_difforigin",
			SLB::FuncCall::create(drawoptions_func_difforigin));
	m.set("drawoptions_func_defaults",
			SLB::FuncCall::create(drawoptions_func_defaults));
	m.set("drawoptions_func_diffall",
			SLB::FuncCall::create(drawoptions_func_diffall));
	{
		const char* code =
				"SLB.drawoptions_func_defaults({})\n";
		lua_assert_valid_dostring(L, code);
	}
	{
		const char* code =
				"SLB.drawoptions_func_difforigin({origin = SLB.CENTER})\n";
		lua_assert_valid_dostring(L, code);
	}
	{
		const char* code =
				"SLB.drawoptions_func_diffall({colour = {1,2,3,4}, region = {1,2,3,4}, origin = SLB.CENTER})\n";
		lua_assert_valid_dostring(L, code);
	}
	lua_close(L);
}
void lua_drawoptions_tests() {
	UNIT_TEST(lua_drawoptions_bind_test);

}

