#include <typeinfo>

#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/testutils.h>

#include "../lua/lua_geometry.h"
#include "../unittest.h"

template<typename T>
void lua_generic_pushget_test(lua_State* L, const T& value) {
	luawrap::push<T>(L, value);
	T fromLua = luawrap::pop<T>(L);

	unit_test_assert(std::string(typeid(value).name()) + " get/set mismatch",
			value == fromLua);

}

static void lua_geo_pushget_test() {
	TestLuaState L;
	lua_register_geometry(L, LuaValue::globals(L));

	lua_generic_pushget_test(L, Posf(0.5, 1.5));
	lua_generic_pushget_test(L, Pos(1, 2));

	lua_generic_pushget_test(L, DimF(0.5, 1.5));
	lua_generic_pushget_test(L, Dim(1, 2));

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

	globals.get(L, "pos_func1") = luawrap::function(L, pos_func1);
	globals.get(L, "pos_func2") = luawrap::function(L, pos_func2);
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

	globals.get(L, "posf_func1") = luawrap::function(L, posf_func1);
	globals.get(L, "posf_func2") = luawrap::function(L, posf_func2);
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

	globals.get(L, "bbox_func") = luawrap::function(L, bbox_func);
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

	globals.get(L, "bboxf_func") = luawrap::function(L, bboxf_func);

	const char* code1 = "bboxf_func({.5, 1.5, 2.5, 3.5})\n";
	lua_assert_valid_dostring(L, code1);
	const char* code2 = "bboxf_func({.5, 1.5, 2.5})\n";
	lua_assert_invalid_dostring(L, code2);
}

static void dim_func(const Dim& dim) {
	UNIT_TEST_ASSERT(dim == Dim(1,2));
}

static void lua_dim_bind_test() {
	TestLuaState L;
	LuaValue globals(L, LUA_GLOBALSINDEX);
	lua_register_geometry(L, globals);

	globals.get(L, "dim_func") = luawrap::function(L, dim_func);

	const char* code1 = "dim_func({1,2})\n";
	lua_assert_valid_dostring(L, code1);
	const char* code2 = "dim_func({1})\n";
	lua_assert_invalid_dostring(L, code2);
}

static void dimf_func(const DimF& dim) {
	UNIT_TEST_ASSERT(dim == DimF(.5,1.5));
}
static void lua_dimf_bind_test() {
	TestLuaState L;
	LuaValue globals(L, LUA_GLOBALSINDEX);
	lua_register_geometry(L, globals);

	globals.get(L, "dimf_func") = luawrap::function(L, dimf_func);

	const char* code1 = "dimf_func({.5,1.5})\n";
	lua_assert_valid_dostring(L, code1);
	const char* code2 = "dimf_func(.5})\n";
	lua_assert_invalid_dostring(L, code2);
}

void lua_geometry_tests() {
	UNIT_TEST(lua_geo_pushget_test);
	UNIT_TEST(lua_pos_bind_test);
	UNIT_TEST(lua_posf_bind_test);
	UNIT_TEST(lua_bbox_bind_test);
	UNIT_TEST(lua_bboxf_bind_test);
	UNIT_TEST(lua_dim_bind_test);
	UNIT_TEST(lua_dimf_bind_test);
}
