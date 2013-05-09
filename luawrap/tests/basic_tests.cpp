#include <UnitTest++.h>

#include <luawrap/testutils.h>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>

template<typename T>
static void luawrap_vector_basic_pushget() {
	TestLuaState L;
	std::vector<T> pushed;

	luawrap::push(L, pushed);

	std::vector<T> got = luawrap::pop<std::vector<T> >(L);

	CHECK(pushed == std::vector<T>());

	L.finish_check();
}

template<typename T>
static void luawrap_number_pushget() {
	TestLuaState L;
	for (int i = -256; i <= 256; i += 256) {
		T pushed = T(i);

		luawrap::push<T>(L, pushed);

		T got = luawrap::pop<T>(L);

		CHECK(pushed == T(i));
	}

	L.finish_check();
}

#define TEMPLATE_TEST(func, temparg) \
	TEST(func##temparg) { func< temparg >(); }

#define TEMPLATE_TEST_NAMED(name, func, temparg) \
	TEST(func##name) { func< temparg >(); }

SUITE (luawrap_basic_tests) {
	TEMPLATE_TEST(luawrap_vector_basic_pushget, int);
	TEMPLATE_TEST(luawrap_number_pushget, float);
	TEMPLATE_TEST(luawrap_number_pushget, double);

	TEMPLATE_TEST(luawrap_number_pushget, int);
	TEMPLATE_TEST_NAMED(luawrap_number_pushgetunsignedint, luawrap_number_pushget, unsigned int);

	TEMPLATE_TEST(luawrap_number_pushget, long);
//	TEMPLATE_TEST_NAMED(luawrap_number_pushgetunsignedlong, luawrap_number_pushget, unsigned long); //known-to-fail, high-range can't always fit in double

	TEMPLATE_TEST(luawrap_number_pushget, char);
	TEMPLATE_TEST_NAMED(luawrap_number_pushget_unsigndchar, luawrap_number_pushget, unsigned char);

#ifndef LUAWRAP_NO_WRAP_LONGLONG
	TEMPLATE_TEST_NAMED(luawrap_number_pushget_longlong, luawrap_number_pushget, long long);
//	TEMPLATE_TEST_NAMED(luawrap_number_pushget_unsignedlonglong, luawrap_number_pushget, unsigned long long); //known-to-fail, high-range can't fit in double
#endif
	TEST(luawrap_string_pushget) {
		TestLuaState L;

		const char* literal = "test";

		/*std::string*/{
			luawrap::push(L, literal);
			CHECK(luawrap::pop<std::string>(L) == literal);
			luawrap::push(L, (char*)literal);
			CHECK(luawrap::pop<std::string>(L) == literal);
			luawrap::push(L, std::string(literal));
			CHECK(luawrap::pop<std::string>(L) == literal);

		}
		/*const char* */{
			luawrap::push(L, literal);
			CHECK(std::string(luawrap::pop<const char*>(L)) == literal);
			luawrap::push(L, (char*)literal);
			CHECK(std::string(luawrap::pop<const char*>(L)) == literal);
			luawrap::push(L, std::string(literal));
			CHECK(std::string(luawrap::pop<const char*>(L)) == literal);
		}
		L.finish_check();
	}
}
