/*
 * lua_image_tests.cpp:
 *  Test ldraw::image bindings in lua
 */

#include <SLB/Manager.hpp>

#include <SLB/FuncCall.hpp>
#include <SLB/LuaCall.hpp>
#include <common/lua/lua_unittest.h>

#include "../Image.h"
#include "../lua/lua_image.h"

static void lua_image_bind_test() {
	using namespace SLB;
	using namespace ldraw;

	lua_State* L = lua_open();
	Image image(NULL, BBoxF(0, 0, 10, 10));

	Manager m;
	m.registerSLB(L);
	m.set("assert", FuncCall::create(unit_test_assert));
	lua_register_image(L);
	{
		const char* code = "function testImgWidth(img)\n"
				"SLB.assert('widths do not match', img.width == 10)\n"
				"SLB.assert('heights do not match', img.height == 10)\n"
				"end\n";

		lua_assert_valid_dostring(L, code);
		LuaCall<void(const Image&)> call(L, "testImgWidth");
		call(image);

	}
	lua_close(L);
}
void lua_image_tests() {
	UNIT_TEST(lua_image_bind_test);

}

