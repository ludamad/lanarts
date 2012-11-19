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
	Manager m;
	m.registerSLB(L);
	//TODO test non-image drawables
	}
	lua_close(L);
}
void lua_drawable_tests() {
	UNIT_TEST(lua_drawable_bind_test);

}

