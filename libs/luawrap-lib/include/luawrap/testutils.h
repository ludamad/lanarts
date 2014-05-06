/*
 * testutils.h:
 *  Utilites for testing luawrap-using code.
 */

#ifndef LUAWRAP_TESTUTILS_H_
#define LUAWRAP_TESTUTILS_H_

struct lua_State;

struct TestLuaState {
	lua_State* L;
	TestLuaState();
	~TestLuaState();
	void finish_check() const;
	operator lua_State*() {
		return L;
	}
};

void lua_assert_valid_dostring(lua_State* L, const char* code);
void lua_assert_invalid_dostring(lua_State* L, const char* code);

#endif /* LUAWRAP_TESTUTILS_H_ */
