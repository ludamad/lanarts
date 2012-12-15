/*
 * lua_image_tests.cpp:
 *  Test ldraw::image bindings in lua
 */

#include <luawrap/testutils.h>
#include <luawrap/LuaValue.h>
#include <luawrap/calls.h>
#include <luawrap/functions.h>

#include <lcommon/unittest.h>

#include "lua_ldraw.h"

#include "Image.h"
#include "lua_font.h"

static void lua_font_bind_test() {
	using namespace ldraw;

	TestLuaState L;
	LuaValue globals = luawrap::globals(L);

	Font f;
	lua_register_font(L, globals);

	luawrap::push<Font>(L, f);
	UNIT_TEST_ASSERT(lua_isuserdata(L, -1));
	lua_pop(L, 1);

	globals["assert"] = luawrap::function(L, unit_test_assert);

	//Cant really test methods, just assert they exist:
	const char* code2 =
			"function testFontProperties(font)\n"
					"assert('draw does not exist', font.draw ~= nil)\n"
					"assert('draw_wrapped does not exist', font.draw_wrapped ~= nil)\n"
					"assert('get_draw_size does not exist', font.get_draw_size ~= nil)\n"
					"end\n";

	lua_assert_valid_dostring(L, code2);
	globals["testFontProperties"].push();
	luawrap::call<void>(L, f);

	L.finish_check();
}

void lua_font_tests() {
	UNIT_TEST(lua_font_bind_test);

}

