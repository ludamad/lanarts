/*
 * lua_image_tests.cpp:
 *  Test ldraw::image bindings in lua
 */

#include <lua.hpp>

#include <common/unittest.h>

#include <luawrap/testutils.h>
#include <luawrap/LuaValue.h>
#include <luawrap/calls.h>
#include <luawrap/functions.h>

#include "Image.h"
#include "Drawable.h"
#include "lua_ldraw.h"

static void lua_drawable_bind_test() {
	using namespace ldraw;

	TestLuaState L;

	Image image;
	image.draw_region() = BBoxF(0, 0, 10, 10);
	Drawable drawable(new Image(image));

	LuaValue globals = luawrap::globals(L);

	globals["assert"] = luawrap::function(L, unit_test_assert);

	const char* code2 = "function testImgProperties(img)\n"
			"assert('width does not exist', img.width ~= nil)\n"
			"assert('height does not exist', img.height ~= nil)\n"
			"assert('draw does not exist', img.draw ~= nil)\n"
			"assert('size does not exist', img.size ~= nil)\n"
			"assert('duration is not nil', img.duration == nil)\n"
			"assert('animated is not false', img.animated == false)\n"
			"end\n";

	lua_assert_valid_dostring(L, code2);
	{
		globals["testImgProperties"].push();
		luawrap::call<void>(L, image);
	}
	//Drawable should be equivalent
	{
		globals["testImgProperties"].push();
		luawrap::call<void>(L, drawable);
	}

	L.finish_check();
}

#include "LuaDrawable.h"

static void lua_luadrawable_bind_test() {
	using namespace ldraw;

	TestLuaState L;

	LuaValue globals = luawrap::globals(L);

	globals["assert"] = luawrap::function(L, unit_test_assert);

	const char* code = "function drawfunc() end";
	lua_assert_valid_dostring(L, code);

	lua_getglobal(L, "drawfunc");
	UNIT_TEST_ASSERT(lua_checkluadrawable(L, -1));
	UNIT_TEST_ASSERT(lua_checkdrawable(L, -1));

	LuaDrawable ldrawable1(L, LuaValue(L, -1));
	LuaDrawable ldrawable2 = luawrap::get<LuaDrawable>(L, -1);
	Drawable drawable = luawrap::get<Drawable>(L, -1);
	LuaDrawable ldrawable3 = *dynamic_cast<LuaDrawable*>(drawable.ptr());
	UNIT_TEST_ASSERT(ldrawable1 == ldrawable2);
	UNIT_TEST_ASSERT(ldrawable1 == ldrawable3);
	lua_pop(L, 1);

	L.finish_check();
}

void lua_drawable_tests() {
	UNIT_TEST(lua_drawable_bind_test);
	UNIT_TEST(lua_luadrawable_bind_test);
}

