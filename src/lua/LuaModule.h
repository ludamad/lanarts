#ifndef LUAMODULE_H_
#define LUAMODULE_H_

namespace YAML {
	class Node;
}

struct lua_State;

class LuaModule {
public:
	LuaModule(lua_State* L) : L(L){
	}
	void initialize(const char* name);
	void deinitialize();
	void set_function(const char* key, lua_CFunction value);
	void set_number(const char* key, double value);
	void set_newtable(const char* key);
	void set_yaml(const char* key, const YAML::Node* root);
	void pop(const char* key);
	void push(const char* key);
private:
	lua_State* L;
};

#endif /* LUAMODULE_H_ */
