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
	LuaValue(lua_State* L, const std::string& expr);
	LuaValue(lua_State* L);
	~LuaValue();

	void initialize();
	void push();
	void pop();
	bool empty() { return impl == NULL; }

	void set_function(const char* key, lua_CFunction value);
	void set_number(const char* key, double value);
	void set_newtable(const char* key);
	void set_yaml(const char* key, const YAML::Node* root);
private:
	lua_State* L;
	LuaValueImpl* impl;
};

#endif /* LUAVALUE_H_ */
