/**
 * LuaField.h:
 *  Represents a value inside another lua object.
 *  This is represented by an index chain of sorts and *MUST* be stored on the stack.
 *
 *  It should NOT be used as a class member. It primarily exists as a proxy convenience class.
 *
 *  You have been warned.
 */

#ifndef LUAFIELD_H_
#define LUAFIELD_H_

struct lua_State;

class LuaStackValue;

class LuaField {
private:
	lua_State* L;
	char _index_type; // 0 for int, 1 for string
	char _parent_type; // 0 for registry, 1 for field, 2 for stack

	/* 2 ways of indexing the parent object */
	union Index {
		int integer;
		const char* string;
	};
	Index _index;

	/* 3 ways of finding the parent object*/
	union Parent {
		void* registry;
		LuaField* field;
		LuaStackValue* stack;
	};

	Parent _parent;
};



#endif /* LUAFIELD_H_ */
