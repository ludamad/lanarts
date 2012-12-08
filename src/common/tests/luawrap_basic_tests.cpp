#include "../lua/lua_unittest.h"

#include "../luawrap/luawrap.h"
#include "../luawrap/functions.h"

template<typename T>
static void luawrap_vector_basic_pushget() {
	TestLuaState L;
	std::vector<T> pushed;

	luawrap::push(L, pushed);

	std::vector<T> got = luawrap::pop<std::vector<T> >(L);

	UNIT_TEST_ASSERT(pushed == std::vector<T>());
}

template<typename T>
static void luawrap_number_pushget() {
	TestLuaState L;
	for (int i = -256; i <= 256; i += 256) {
		T pushed = T(i);

		luawrap::push<T>(L, pushed);

		T got = luawrap::pop<T>(L);

		UNIT_TEST_ASSERT(pushed == T(i));
	}
}

static void luawrap_string_pushget() {
	TestLuaState L;

	const char* literal = "test";

	/*std::string*/{
		luawrap::push(L, literal);
		UNIT_TEST_ASSERT(luawrap::pop<std::string>(L) == literal);
		luawrap::push(L, (char*)literal);
		UNIT_TEST_ASSERT(luawrap::pop<std::string>(L) == literal);
		luawrap::push(L, std::string(literal));
		UNIT_TEST_ASSERT(luawrap::pop<std::string>(L) == literal);

	}
	/*const char* */{
		luawrap::push(L, literal);
		UNIT_TEST_ASSERT(std::string(luawrap::pop<const char*>(L)) == literal);
		luawrap::push(L, (char*)literal);
		UNIT_TEST_ASSERT(std::string(luawrap::pop<const char*>(L)) == literal);
		luawrap::push(L, std::string(literal));
		UNIT_TEST_ASSERT(std::string(luawrap::pop<const char*>(L)) == literal);
	}
}

static void luawrap_basic_pushget() {
	UNIT_TEST(luawrap_vector_basic_pushget<int>);

	UNIT_TEST(luawrap_number_pushget<float>);
	UNIT_TEST(luawrap_number_pushget<double>);

	UNIT_TEST(luawrap_number_pushget<int>);
	UNIT_TEST(luawrap_number_pushget<unsigned int>);

	UNIT_TEST(luawrap_number_pushget<long>);
	//UNIT_TEST(luawrap_number_pushget<unsigned long>); //known-to-fail, high-range can't always fit in double

	UNIT_TEST(luawrap_number_pushget<char>);
	UNIT_TEST(luawrap_number_pushget<unsigned char>);

#ifndef LUAWRAP_NO_WRAP_LONGLONG
	UNIT_TEST(luawrap_number_pushget<long long>);
//	UNIT_TEST(luawrap_number_pushget<unsigned long long>); //known-to-fail, high-range can't fit in double
#endif
	UNIT_TEST(luawrap_string_pushget);
}

void luawrap_tests() {
	luawrap_basic_pushget();
}
