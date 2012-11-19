/*
 * lua_image_tests.cpp:
 *  Test ldraw::image bindings in lua
 */

#include <SLB/Manager.hpp>

#include <SLB/FuncCall.hpp>
#include <SLB/LuaCall.hpp>
#include <common/lua/lua_unittest.h>

#include "../Image.h"
#include "../lua/lua_font.h"

static void lua_font_bind_test() {
	using namespace SLB;
	using namespace ldraw;

	lua_State* L = lua_open();
	{
		Font f;
		Manager m;
		m.registerSLB(L);
		lua_register_font(L);
		m.set("assert", FuncCall::create(unit_test_assert));

		//Cant really test methods, just assert they exist:
		const char* code2 = "function testFontProperties(font)\n"
				"SLB.assert('draw does not exist', font.draw ~= nil)\n"
				"SLB.assert('draw_wrapped does not exist', font.draw_wrapped ~= nil)\n"
				"SLB.assert('get_draw_size does not exist', font.get_draw_size ~= nil)\n"
				"end\n";

		lua_assert_valid_dostring(L, code2);
		{
			LuaCall<void(const Font&)> call(L, "testFontProperties");
			call(f);
		}
	}
	lua_close(L);
}

void lua_font_tests() {
	UNIT_TEST(lua_font_bind_test);

}

