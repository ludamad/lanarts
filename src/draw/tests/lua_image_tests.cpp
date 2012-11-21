/*
 * lua_image_tests.cpp:
 *  Test ldraw::image bindings in lua
 */

#include <common/unittest.h>
#include <common/lua/lua_unittest.h>
#include <SLB/LuaCall.hpp>

#include "../Image.h"

#include "../lua/lua_image.h"

//#include <SLB/Manager.hpp>
//
//#include <SLB/FuncCall.hpp>
//#include <SLB/LuaCall.hpp>
//#include <common/lua/lua_unittest.h>
//#include <common/lua/LuaValue.h>
//#include "../lua/lua_ldraw.h"
//
//#include "../Image.h"
//#include "../lua/lua_image.h"
//#include "../lua/lua_drawable.h"

static void SLB_sanity_test() {
	using namespace SLB;

	lua_State* L = lua_open();

	{
		Manager m;
		m.registerSLB(L);
//		ldraw::lua_register_image(L, LuaValue(L, LUA_GLOBALSINDEX));
		const char* code = "function checksanity()\n"
				"end\n";

		lua_assert_valid_dostring(L, code);
		{ //NB: scope HAS to end before lua_Close
			LuaCall<void()> call(L, "checksanity");
			call(); // Smoke test
		}
		{
			int i;
			LuaCall<void(int)> call(L, "checksanity");
			call(i); // Smoke test
		}
		{
			using namespace ldraw;
			Image image;
			LuaCall<void(const Image&)> call(L, "checksanity");
			call(image); // Smoke test
		}
	}

	lua_close(L);
}
//
//static void lua_image_bind_test() {
//	using namespace SLB;
//	using namespace ldraw;
//
//	lua_State* L = lua_open();
//	{
//		Image image;
//		image.draw_region() = BBoxF(0, 0, 10, 10);
//		Drawable drawable(new Image(image));
//
//		Manager m;
//		m.registerSLB(L);
//		m.set("assert", FuncCall::create(unit_test_assert));
//
//		{
//			const char* code2 =
//					"function testImgProperties(img)\n"
//							"SLB.assert('width does not exist', img.width ~= nil)\n"
//							"SLB.assert('height does not exist', img.height ~= nil)\n"
//							"SLB.assert('draw does not exist', img.draw ~= nil)\n"
//							"SLB.assert('size does not exist', img.size ~= nil)\n"
//							"SLB.assert('duration is not nil', img.duration == nil)\n"
//							"SLB.assert('animated is not false', img.animated == false)\n"
//							"end\n";
//
//			lua_assert_valid_dostring(L, code2);
//			{
//				LuaCall<void(const Image&)> call(L, "testImgProperties");
//				call(image);
//			}
//			//Drawable should be equivalent
//			{
//				LuaCall<void(const Drawable&)> call(L, "testImgProperties");
//				call(drawable);
//			}
//		}
//		{
//			const char* code =
//					"function testImgWidth(img)\n"
//							"SLB.assert('widths do not match', img.width == 10)\n"
//							"SLB.assert('heights do not match', img.height == 10)\n"
//							"SLB.assert('duration is not nil', img.duration == nil)\n"
//							"SLB.assert('animated is not false', img.animated == false)\n"
//							"end\n";
//
//			lua_assert_valid_dostring(L, code);
//			{
//				LuaCall<void(const Image&)> call(L, "testImgWidth");
//				call(image);
//			}
//			//Drawable should be equivalent
//			{
//				LuaCall<void(const Drawable&)> call(L, "testImgWidth");
//				call(drawable);
//			}
//		}
//
//	}
//	lua_close(L);
//}

void lua_image_tests() {
	UNIT_TEST(SLB_sanity_test);
//	UNIT_TEST(lua_image_bind_test);

}

