/*
 * lua_image_tests.cpp:
 *  Test ldraw::image bindings in lua
 */

#include <SLB/Manager.hpp>

#include <SLB/FuncCall.hpp>
#include <SLB/LuaCall.hpp>
#include <common/lua/lua_unittest.h>

#include "../Image.h"
#include "../lua/lua_drawable.h"

static void lua_drawable_bind_test() {
	using namespace SLB;
	using namespace ldraw;

	lua_State* L = lua_open();
	{
	Image image;
	image.draw_region() = BBoxF(0, 0, 10, 10);
	Manager m;
	m.registerSLB(L);
	Drawable drawable(new Image(image));

	m.set("assert", FuncCall::create(unit_test_assert));

	{
		const char* code2 = "function testImgProperties(img)\n"
				"SLB.assert('width does not exist', img.width ~= nil)\n"
				"SLB.assert('height does not exist', img.height ~= nil)\n"
				"SLB.assert('draw does not exist', img.draw ~= nil)\n"
				"SLB.assert('size does not exist', img.size ~= nil)\n"
				"SLB.assert('duration is not nil', img.duration == nil)\n"
				"SLB.assert('animated is not false', img.animated == false)\n"
				"end\n";

		lua_assert_valid_dostring(L, code2);
		{
			LuaCall<void(const Image&)> call(L, "testImgProperties");
			call(image);
		}
		//Drawable should be equivalent
		{
			LuaCall<void(const Drawable&)> call(L, "testImgProperties");
			call(drawable);
		}
	}
	}
	lua_close(L);
}
void lua_drawable_tests() {
	UNIT_TEST(lua_drawable_bind_test);

}

