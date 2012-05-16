/*
 * LuaValue.h
 *
 *  Created on: May 16, 2012
 *      Author: adomurad
 */

#ifndef LUAVALUE_H_
#define LUAVALUE_H_

#include <string>

class LuaValue {
public:
	void initialize();
	void deinitialize();
	void push();
	LuaValue(const std::string& expr);
	~LuaValue();
private:
	std::string lua_expression;
	lua_State* L;
};

#endif /* LUAVALUE_H_ */
