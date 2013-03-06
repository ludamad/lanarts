/**
 * LuaField.h:
 *	PLEASE READ BEFORE USING.
 *
 *  Represents a value inside another lua object.
 *  This is represented by an index chain of sorts and *MUST* be stored on the stack.
 *  There is some trade-off of usability and performance here, but generally it is very fast.
 *
 *  It should NOT be used as a class member. It primarily exists as a proxy convenience class.
 *
 *  You have been warned.
 */

#ifndef LUAFIELD_H_
#define LUAFIELD_H_

#include <string>

struct lua_State;

class LuaStackValue;

/* Represents a field inside an object.
 * ONLY FOR STACK USE. Please read notes above before using! */
class LuaField {
public:
	/** Constructors **/

	/* Looks up an object in the registry */
	LuaField(lua_State* L, void* parent, const char* index);
	LuaField(lua_State* L, void* parent, int index);

	/* Looks up another LuaField */
	LuaField(LuaField* parent, const char* index);
	LuaField(LuaField* parent, int index);

	/* Looks up a lua stack value */
	LuaField(lua_State* L, int stackidx, const char* index);
	LuaField(lua_State* L, int stackidx, int index);

	/* Returning path to object */
	std::string index_path() const;
	void index_path(std::string& str) const;

	/** Utility methods **/
	void push() const;
	void pop() const;

private:
	void handle_nil_parent() const;
	void push_parent() const;

	union Index {
		int integer;
		const char* string;
	};

	union Parent {
		LuaField* field;
		void* registry;
		int stack_index;
	};

	void init(lua_State* L, char index_type, char parent_type, Index index, Parent parent);

	/* members */
	lua_State* L;
	char _index_type; // 0 for int, 1 for string
	char _parent_type; // 0 for field, 1 for registry, 2 for stack

	Index _index;
	Parent _parent;
};



#endif /* LUAFIELD_H_ */
