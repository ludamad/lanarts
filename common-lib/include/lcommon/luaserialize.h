/*
 * Adapted from lmarshal.h:
 * A Lua library for serializing and deserializing Lua values
 * Richard Hundt <richardhundt@gmail.com>
 *
 * License: MIT
 *
 * Copyright (c) 2010 Richard Hundt
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LCOMMON_LUA_SERIALIZE_H_
#define LCOMMON_LUA_SERIALIZE_H_

#include <luawrap/LuaValue.h>

// TODO: Its amazing that the global object naming even works but
// there is some mess to clean after my vicious hacking

class SerializeBuffer;
struct lua_State;
class LuaValue;

struct LuaSerializeContext {
	lua_State* L;
	SerializeBuffer* buffer;
	// Lua values that control the serialization.
	// Note, these indices will not 'survive' a lua stack call barrier.
	int obj_to_index, index_to_obj, index_failure_function;
	// Passed to each serialization function
	int context_object;
	size_t next_encode_index, next_decode_index;

	LuaSerializeContext(lua_State* L, SerializeBuffer* buffer,
			int obj_to_index, int index_to_obj,
			int index_failure_function,
			int context_object) {
		this->L = L;
		this->buffer = buffer;
		this->next_decode_index = lua_objlen(L, this->index_to_obj) + 1;
		this->next_encode_index = lua_objlen(L, this->obj_to_index) + 1;
		this->obj_to_index = obj_to_index;
		this->index_to_obj = index_to_obj;
		this->index_failure_function = index_failure_function;
		this->context_object = context_object;
	}

	// Captures the top 4 indices, without popping them.
	// Note, these indices will not 'survive' a lua stack call barrier.
	static LuaSerializeContext ref_top4(lua_State* L, SerializeBuffer* buffer) {
		int top = lua_gettop(L);
		return LuaSerializeContext(L, buffer, top - 3, top - 2, top - 1, top);
	}

	// Convenient default context for simple serialization
	static LuaSerializeContext push3(lua_State* L, SerializeBuffer* buffer) {
		lua_newtable(L); // obj_to_index
		lua_newtable(L); // index_to_obj
		lua_newtable(L); // context_objext
		int top = lua_gettop(L);
		return LuaSerializeContext(L, buffer, top - 2, top - 1, -1, top);
	}

	// Encode a value from the stack.
	void encode(int idx);
	// Decode and push a Lua value.
	void decode();

	void encode(const LuaValue& value);
	void decode(LuaValue& value);
private:
	// Decode a Lua value, passing its type explicitly. Allows for peeking at the type.
	void _decode(int type);
	void store_ref_id(int idx);
	// Encodes the object ref on succcess
	bool test_has_ref(int idx);
	// Encodes using an object method on success
	bool test_has_metamethod(int idx);
	void encode_metatable(int idx);
	void encode_table(int idx);
	void encode_function(int idx);

	void decode_table();
	void decode_function();
	void decode_string();
	void decode_ref_name();
	void decode_ref_metamethod();
};

// Metatable-based class wrapped for SerializeBuffer in Lua.
// Makes it possible to push SerializeBuffer's with luawrap.
LuaValue lua_serializebuffer_type(lua_State *L);

void luaserialize_encode(lua_State* L, SerializeBuffer& serializer, int idx);
void luaserialize_decode(lua_State* L, SerializeBuffer& serializer);

struct LuaSerializeConfig {
	lua_State* L;
	LuaValue obj_to_index, index_to_obj;
	LuaValue index_failure_function, context_object;
	LuaSerializeConfig(lua_State* L) {
		this->L = L;
		obj_to_index = LuaValue::newtable(L);
		index_to_obj = LuaValue::newtable(L);
		index_failure_function.init(L);
		context_object = LuaValue::newtable(L);
	}
	LuaSerializeContext push4(SerializeBuffer& serializer);
	void encode(SerializeBuffer& serializer, int idx);
	void decode(SerializeBuffer& serializer);
	void encode(SerializeBuffer& serializer, const LuaValue& value);
	void decode(SerializeBuffer& serializer, LuaValue& value);
};

void luaserialize_encode(lua_State* L, SerializeBuffer& serializer, const LuaValue& value);
void luaserialize_decode(lua_State* L, SerializeBuffer& serializer, LuaValue& value);

#endif /* LCOMMON_LUA_SERIALIZE_H_ */
