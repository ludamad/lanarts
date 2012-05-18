#ifndef LUAVALUE_H_
#define LUAVALUE_H_

#include <string>

namespace YAML {
	class Node;
}

class LuaValueImpl;
struct lua_State;

typedef int (*lua_CFunction) (lua_State *L);

class LuaValue {
public:
	LuaValue(const std::string& expr);
	LuaValue(const LuaValue& value);
	LuaValue();
	~LuaValue();


	void operator=(const LuaValue& value);

	void initialize(lua_State* L);
	void deinitialize(lua_State* L);
	void push(lua_State* L);
	void pop(lua_State* L);
	bool empty();

	void table_initialize(lua_State* L);
	void set_function(lua_State* L, const char* key, lua_CFunction value);
	void set_number(lua_State* L, const char* key, double value);
	void set_newtable(lua_State* L, const char* key);
	void set_yaml(lua_State* L, const char* key, const YAML::Node* root);
private:
	LuaValueImpl* impl;
};

void lua_gameinstcallback(lua_State* L, LuaValue& value, int id);

#endif /* LUAVALUE_H_ */
