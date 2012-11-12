#include <typeinfo>

#include <SLB/Manager.hpp>
#include <SLB/Script.hpp>

#include "../lua/lua_unittest.h"

#include "../lua/lua_geometry.h"

template<typename T>
void lua_generic_pushget_test(lua_State* L, const T& value) {
	SLB::push<T>(L, value);
	T fromLua = SLB::get<T>(L, -1);

	unit_test_assert(std::string(typeid(value).name()) + " get/set mismatch",
			value == fromLua);

}
static void lua_geo_pushget_test() {
	lua_State* L = lua_open();

	lua_generic_pushget_test(L, Posf(0.5, 1.5));
	lua_generic_pushget_test(L, Pos(1, 2));

	lua_generic_pushget_test(L, DimF(0.5, 1.5));
	lua_generic_pushget_test(L, Dim(1, 2));

	lua_generic_pushget_test(L, BBoxF(0.5, 1.5, 2.5, 3.5));
	lua_generic_pushget_test(L, BBox(1, 2, 3, 4));

	lua_close(L);
}

static void pos_func1(Pos p) {
	UNIT_TEST_ASSERT(p == Pos(1,2));
}
static void pos_func2(const Pos& p) {
	UNIT_TEST_ASSERT(p == Pos(1,2));
}
static void posf_func1(Posf p) {
	UNIT_TEST_ASSERT(p == Posf(.5,1.5));
}
static void posf_func2(const Posf& p) {
	UNIT_TEST_ASSERT(p == Posf(.5,1.5));
}

static void bbox_func(const BBox& bbox) {
	UNIT_TEST_ASSERT(bbox == BBox(1,2,3,4));
}
static void bboxf_func(const BBoxF& bbox) {
	UNIT_TEST_ASSERT(bbox == BBoxF(.5,1.5,2.5,3.5));
}

static void dim_func(const Dim& dim) {
	UNIT_TEST_ASSERT(dim == Dim(1,2));
}

static void dimf_func(const DimF& dim) {
	UNIT_TEST_ASSERT(dim == DimF(.5,1.5));
}

static void lua_geo_bind_test() {
	lua_State* L = lua_open();
	luaL_openlibs(L);

	SLB::Manager m;
	m.set("pos_func1", SLB::FuncCall::create(pos_func1));
	m.set("pos_func2", SLB::FuncCall::create(pos_func2));

	m.set("posf_func1", SLB::FuncCall::create(posf_func1));
	m.set("posf_func2", SLB::FuncCall::create(posf_func2));

	m.set("bbox_func", SLB::FuncCall::create(bbox_func));
	m.set("bboxf_func", SLB::FuncCall::create(bboxf_func));

	m.set("dim_func", SLB::FuncCall::create(dim_func));
	m.set("dimf_func", SLB::FuncCall::create(dimf_func));

	m.registerSLB(L);
	{
		const char* code = "SLB.pos_func1({1,2})\n"
				"SLB.pos_func2({1,2})\n";
		lua_assert_valid_dostring(L, code);
	}
	{

		const char* code = "SLB.posf_func1({.5,1.5})\n"
				"SLB.posf_func2({.5,1.5})\n";
		lua_assert_valid_dostring(L, code);
	}

	{
		const char* code1 = "SLB.bbox_func({1,2,3,4})\n";
		lua_assert_valid_dostring(L, code1);
		const char* code2 = "SLB.bbox_func({1,2,3})\n";
		lua_assert_invalid_dostring(L, code2);
	}
	{
		const char* code1 = "SLB.bboxf_func({.5, 1.5, 2.5, 3.5})\n";
		lua_assert_valid_dostring(L, code1);
		const char* code2 = "SLB.bboxf_func({.5, 1.5, 2.5})\n";
		lua_assert_invalid_dostring(L, code2);
	}

	{
		const char* code1 = "SLB.dim_func({1,2})\n";
		lua_assert_valid_dostring(L, code1);
		const char* code2 = "SLB.dim_func({1})\n";
		lua_assert_invalid_dostring(L, code2);
	}
	{
		const char* code1 = "SLB.dimf_func({.5,1.5})\n";
		lua_assert_valid_dostring(L, code1);
		const char* code2 = "SLB.dimf_func(.5})\n";
		lua_assert_invalid_dostring(L, code2);
	}

	lua_close(L);
}

void lua_geometry_tests() {
	UNIT_TEST(lua_geo_pushget_test);
	UNIT_TEST(lua_geo_bind_test);
}
