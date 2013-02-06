#include <typeinfo>

#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/testutils.h>

#include "lua_geometry.h"
#include "unittest.h"

template<typename T>
void lua_generic_pushget_test(lua_State* L, const T& value) {
	luawrap::push<T>(L, value);
	T fromLua = luawrap::pop<T>(L);

	unit_test_assert(std::string(typeid(value).name()) + " get/set mismatch",
			value == fromLua);

}

static void lua_geo_pushget_test() {
	TestLuaState L;
	lua_register_geometry(L, luawrap::globals(L));

	lua_generic_pushget_test(L, Posf(0.5, 1.5));
	lua_generic_pushget_test(L, Pos(1, 2));

	lua_generic_pushget_test(L, SizeF(0.5, 1.5));
	lua_generic_pushget_test(L, Size(1, 2));

	lua_generic_pushget_test(L, BBoxF(0.5, 1.5, 2.5, 3.5));
	lua_generic_pushget_test(L, BBox(1, 2, 3, 4));
}

static void pos_func1(Pos p) {
	UNIT_TEST_ASSERT(p == Pos(1,2));
}
static void pos_func2(const Pos& p) {
	UNIT_TEST_ASSERT(p == Pos(1,2));
}

static void lua_pos_bind_test() {
	TestLuaState L;
	LuaValue globals(L, LUA_GLOBALSINDEX);
	lua_register_geometry(L, globals);

	globals["pos_func1"].bind_function(pos_func1);
	globals["pos_func2"].bind_function(pos_func2);

	const char* code = "pos_func1({1,2})\n"
			"pos_func2({1,2})\n";
	lua_assert_valid_dostring(L, code);
}

static void posf_func1(Posf p) {
	UNIT_TEST_ASSERT(p == Posf(.5,1.5));
}
static void posf_func2(const Posf& p) {
	UNIT_TEST_ASSERT(p == Posf(.5,1.5));
}

static void lua_posf_bind_test() {
	TestLuaState L;
	LuaValue globals(L, LUA_GLOBALSINDEX);
	lua_register_geometry(L, globals);

	globals["posf_func1"].bind_function(posf_func1);
	globals["posf_func2"].bind_function(posf_func2);

	const char* code = "posf_func1({.5,1.5})\n"
			"posf_func2({.5,1.5})\n";
	lua_assert_valid_dostring(L, code);
}

static void bbox_func(const BBox& bbox) {
	UNIT_TEST_ASSERT(bbox == BBox(1,2,3,4));
}

static void lua_bbox_bind_test() {
	TestLuaState L;
	LuaValue globals(L, LUA_GLOBALSINDEX);
	lua_register_geometry(L, globals);

	globals["bbox_func"].bind_function(bbox_func);
	const char* code1 = "bbox_func({1,2,3,4})\n";
	lua_assert_valid_dostring(L, code1);
	const char* code2 = "bbox_func({1,2,3})\n";
	lua_assert_invalid_dostring(L, code2);
}

static void bboxf_func(const BBoxF& bbox) {
	UNIT_TEST_ASSERT(bbox == BBoxF(.5,1.5,2.5,3.5));
}

static void lua_bboxf_bind_test() {
	TestLuaState L;
	LuaValue globals(L, LUA_GLOBALSINDEX);
	lua_register_geometry(L, globals);

	globals["bboxf_func"].bind_function(bboxf_func);

	const char* code1 = "bboxf_func({.5, 1.5, 2.5, 3.5})\n";
	lua_assert_valid_dostring(L, code1);
	const char* code2 = "bboxf_func({.5, 1.5, 2.5})\n";
	lua_assert_invalid_dostring(L, code2);
}

static void size_func(const Size& size) {
	UNIT_TEST_ASSERT(size == Size(1,2));
}

static void lua_size_bind_test() {
	TestLuaState L;
	LuaValue globals(L, LUA_GLOBALSINDEX);
	lua_register_geometry(L, globals);

	globals["size_func"].bind_function(size_func);

	const char* code1 = "size_func({1,2})\n";
	lua_assert_valid_dostring(L, code1);
	const char* code2 = "size_func({1})\n";
	lua_assert_invalid_dostring(L, code2);
}

static void sizef_func(const SizeF& size) {
	UNIT_TEST_ASSERT(size == SizeF(.5,1.5));
}
static void lua_sizef_bind_test() {
	TestLuaState L;
	LuaValue globals(L, LUA_GLOBALSINDEX);
	lua_register_geometry(L, globals);

	globals["sizef_func"].bind_function(sizef_func);

	const char* code1 = "sizef_func({.5,1.5})\n";
	lua_assert_valid_dostring(L, code1);
	const char* code2 = "sizef_func(.5})\n";
	lua_assert_invalid_dostring(L, code2);
}

void lua_geometry_tests() {
	UNIT_TEST(lua_geo_pushget_test);
	UNIT_TEST(lua_pos_bind_test);
	UNIT_TEST(lua_posf_bind_test);
	UNIT_TEST(lua_bbox_bind_test);
	UNIT_TEST(lua_bboxf_bind_test);
	UNIT_TEST(lua_size_bind_test);
	UNIT_TEST(lua_sizef_bind_test);
}
