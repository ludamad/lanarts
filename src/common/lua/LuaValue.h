/**
 * LuaValue.h:
 *  Represents a value in the lua registry.
 *  This is a table in lua_State* that cannot be altered by lua code, and thus
 *  can safely be used for internal purposes.
 */

#ifndef LCOMMON_LUAVALUE_H_
#define LCOMMON_LUAVALUE_H_

#include <string>

class LuaValueImpl;
struct lua_State;
class SerializeBuffer;

typedef int (*lua_CFunction)(lua_State *L);

class LuaValue {
public:
	LuaValue(const std::string& expr);
	LuaValue(const LuaValue& value);
	LuaValue(lua_State* L, int pos);
	LuaValue();
	~LuaValue();

	void operator=(const LuaValue& value);

	void initialize(lua_State* L);
	void deinitialize(lua_State* L);
	void push(lua_State* L) const;
	void pop(lua_State* L);
	void set(lua_State* L, int pos);
	bool empty() const;

	void table_initialize(lua_State* L);
	void table_pop_value(lua_State* L, const char* key);
	void table_push_value(lua_State* L, const char* key);
	void table_set_function(lua_State* L, const char* key, lua_CFunction value);

	void table_set_newtable(lua_State* L, const char* key);
	/* For convenience, since these keys will often be dynamically allocated 'string's */
	void table_push_value(lua_State* L, const std::string& key) {
		table_push_value(L, key.c_str());
	}
	void serialize(lua_State* L, SerializeBuffer& serializer);
	void deserialize(lua_State* L, SerializeBuffer& serializer);

	bool isnil(lua_State* L);
private:
	LuaValueImpl* impl;
};

#endif /* LCOMMON_LUAVALUE_H_ */
