/*
 * lua_drawoptions_tests.cpp:
 *  Test drawoptions bindings in lua
 */

#include <luawrap/testutils.h>
#include <luawrap/LuaValue.h>
#include <luawrap/calls.h>
#include <luawrap/functions.h>

#include <lcommon/unittest.h>

#include "DrawOptions.h"

#include "lua_ldraw.h"

SUITE(lua_drawoptions_tests) {

	static void drawoptions_func_defaults(const ldraw::DrawOptions& options) {
		ldraw::DrawOptions defaults;
		UNIT_TEST_ASSERT(options.draw_colour == defaults.draw_colour);
		UNIT_TEST_ASSERT(options.draw_origin == defaults.draw_origin);
		UNIT_TEST_ASSERT(options.draw_region == defaults.draw_region);
		UNIT_TEST_ASSERT(options.draw_scale == defaults.draw_scale);
		UNIT_TEST_ASSERT(options.draw_angle == defaults.draw_angle);
		UNIT_TEST_ASSERT(options.draw_frame == defaults.draw_frame);
	}
	static void drawoptions_func_difforigin(const ldraw::DrawOptions& options) {
		ldraw::DrawOptions defaults;
		UNIT_TEST_ASSERT(options.draw_colour == defaults.draw_colour);
		UNIT_TEST_ASSERT(options.draw_origin == ldraw::CENTER);
		UNIT_TEST_ASSERT(options.draw_region == defaults.draw_region);
		UNIT_TEST_ASSERT(options.draw_scale == defaults.draw_scale);
		UNIT_TEST_ASSERT(options.draw_angle == defaults.draw_angle);
		UNIT_TEST_ASSERT(options.draw_frame == defaults.draw_frame);
	}

	static void drawoptions_func_diffall(const ldraw::DrawOptions& options) {
		UNIT_TEST_ASSERT(options.draw_colour == Colour(1,2,3,4));
		UNIT_TEST_ASSERT(options.draw_origin == ldraw::CENTER);
		UNIT_TEST_ASSERT(options.draw_region == BBoxF(1,2,3,4));
		UNIT_TEST_ASSERT(options.draw_scale == SizeF(1,2));
		UNIT_TEST_ASSERT(options.draw_angle == 1.0f);
		UNIT_TEST_ASSERT(options.draw_frame == 1.0f);
	}

	TEST(lua_drawoptions_bind_test) {
		using namespace ldraw;

		TestLuaState L;
		LuaValue globals = luawrap::globals(L);

		lua_register_ldraw(L, globals);

		globals["drawoptions_func_difforigin"] = luawrap::function(L,
				drawoptions_func_difforigin);

		globals["drawoptions_func_defaults"] = luawrap::function(L,
				drawoptions_func_defaults);

		globals["drawoptions_func_diffall"] = luawrap::function(L,
				drawoptions_func_diffall);

		{
			const char* code = "drawoptions_func_defaults({})\n";
			lua_assert_valid_dostring(L, code);
		}
		{
			const char* code =
					"drawoptions_func_difforigin({origin = CENTER})\n";
			lua_assert_valid_dostring(L, code);
		}
		{
			const char* code =
					"drawoptions_func_diffall({color = {1,2,3,4}, region = {1,2,3,4}, "
							" origin = CENTER, scale = {1,2}, angle = 1, frame = 1})\n";
			lua_assert_valid_dostring(L, code);
		}

		L.finish_check();
	}
}

