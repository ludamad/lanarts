#ifndef LUAVALUE_H_
#define LUAVALUE_H_

#include <string>

namespace YAML {
	class Node;
}

class LuaValueImpl;

class LuaValue {
	LuaValue(const std::string& expr);
	LuaValue();
	~LuaValue();

	void initialize();
	void push();
	void pop();

	void set_function(const char* key, lua_CFunction value);
	void set_number(const char* key, double value);
	void set_newtable(const char* key);
	void set_yaml(const char* key, const YAML::Node* root);
private:
	LuaValueImpl* impl;
};

#endif /* LUAVALUE_H_ */
