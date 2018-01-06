/* Loosely based off lmarshal.c. */

#include <stdlib.h>
#include <string.h>

#include <exception>
#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <algorithm>

#include "luaserialize.h"
#include "SerializeBuffer.h"

enum lstype_t {
	LS_NUMBER,
	LS_BOOL,
	LS_NIL,
	LS_STRING,
	LS_FUNCTION,
	LS_TABLE,
	// Marks the end of a table:
	LS_TABLE_END_SENTINEL,
	// For ordinary references (ref by continuously incremented integer):
	LS_REF_ID,
	// For named references (ref by string, eg for globals):
	LS_REF_NAME,
	// Decodes by calling __load on the object's decoded metatable, with decoded arguments:
	LS_REF_METAMETHOD,
	LS_NOVALUE
};


#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : \
                                        lua_gettop(L) + (i) + 1)

static std::vector<std::string> get_keys(const LuaValue& value) {
    if (value.empty()) {
        return {};
    }
    std::vector<std::string> keys;
    value.push();
    lua_State* L = value.luastate();
    if (lua_istable(L, -1)) {
        lua_pushnil(L);  /* first key */
        while (lua_next(L, -2) != 0) {
            /* uses 'key' (at index -2) and 'value' (at index -1) */
            const char* str_rep = nullptr;

            lua_pushvalue(L, -2);
            if (lua_isstring(L, -1)) {
                str_rep = lua_tostring(L, -1);
            } else {
                str_rep = lua_typename(L, lua_type(L, -1));
            }
            keys.push_back({str_rep});
            /* removes copy of key, removes 'value'; keeps 'key' for next iteration */
            lua_pop(L, 2);
        }
    }
    lua_pop(L, 1);
    return keys;
}

static void encode_keys(SerializeBuffer& serializer, const LuaValue& value) {
    // TODO if ndebug
    serializer.write_container(get_keys(value));
}

static void decode_keys(SerializeBuffer& serializer, const LuaValue& value) {
    std::vector<std::string> keys;
    serializer.read_container(keys);
    std::vector<std::string> new_keys = get_keys(value);
    std::sort(keys.begin(), keys.end());
    std::sort(new_keys.begin(), new_keys.end());
//    TODO print path of object?
//                       TODO add object path to keys?
    if (keys != new_keys) {
        throw std::runtime_error("Corrupted load!");
    }
}

/***************************************************************************************
 *                           Lua value encoding                                        *
 ***************************************************************************************/

// On success, writes [LS_TABLE, (key encoding, value encoding)*, LS_TABLE_END_SENTINEL]
void LuaSerializeContext::encode_table(int idx) {
    lua_pushvalue(L, idx);
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
        const char* name = NULL;
        const char* type_name = NULL;
		if (lua_type(L, -2) == LUA_TSTRING) {
			name = lua_tostring(L, -2);
            type_name = lua_typename(L, lua_type(L, -1));
			//printf("ENCODING '%s' as '%s'\n", lua_tostring(L, -2), lua_typename(L, lua_type(L, -1)));
		}
//        LuaValue k{L, -2};
//        LuaValue v{L, -1};
		encode(-2);
//        try {
        encode(-1);
//        } catch (...) {
//            //
//            printf("Error occurred -- dumping Lua object at %d\n", idx);
//            lua_getglobal(L, "pretty_table_safe");
//            k.push();
//            lua_call(L, 1, 0);
//            printf("Value\n");
//            lua_getglobal(L, "pretty_table_safe");
//            v.push();
//            lua_call(L, 1, 0);
//            //
//            throw;
//        };
		lua_pop(L, 1);
	}
	buffer->write_byte(LS_TABLE_END_SENTINEL);
	lua_pop(L, 1);
    {LuaValue VALUE{L, idx};
        encode_keys(*buffer, VALUE);}
}

// On success, writes [LS_REF_ID, <ref id>] or [LS_REF_NAME, <ref name>]
bool LuaSerializeContext::test_has_ref(int idx) {
    idx = abs_index(L, idx);
//	printf("0x%lX %s\n", lua_topointer(L, -1), lua_typename(L, lua_type(L, -1)));
	lua_pushvalue(L, idx);
	lua_rawget(L, this->obj_to_index);
	bool was_seen = !lua_isnil(L, -1);
//	printf("0x%lX %s\n", lua_topointer(L, -2), was_seen ? "was seen" : "wasn't seen");
	if (was_seen) {
		if (lua_type(L, -1) == LUA_TNUMBER) {
			buffer->write_byte(LS_REF_ID);
	        buffer->write_byte(lua_type(L, idx));
			buffer->write_int(lua_tointeger(L, -1));
//			printf("Writing object with integer ID\n");
		} else {
			size_t str_len;
			const char* str = lua_tolstring(L, -1, &str_len);
//			printf("Test: '%s', %d chars\n", str, (int)str_len);
			buffer->write_byte(LS_REF_NAME);
            buffer->write_byte(lua_type(L, idx));
			buffer->write_int(str_len);
			buffer->write_raw(str, str_len);
		}
	}
	lua_pop(L, 1);
	return was_seen;
}

static int lua_serialize_buffer_writer(lua_State* L, const void* data, size_t len, void* buffer) {
	((SerializeBuffer*)buffer)->write_raw((const char*) data, len);
	return 0;
}


#define DEBUG_SERIALIZATION

void LuaSerializeContext::store_ref_id(int idx) {
	//printf("STORING %d AS %s\n", (int)next_encode_index, lua_typename(L, lua_type(L, idx)));
	LCOMMON_ASSERT(!lua_isnil(L, idx));
	lua_pushvalue(L, idx);
	lua_pushinteger(L, next_encode_index++);
	lua_rawset(L, this->obj_to_index);
}

// Writes [LS_FUNCTION, <number of upvalues>, <upvalue encodings>]
void LuaSerializeContext::encode_function(int idx) {
    idx = abs_index(L, idx);
        //printf("ENCODING FUNC %d\n", next_encode_index);
	lua_pushvalue(L, idx); // push function

	lua_Debug ar;
	lua_getinfo(L, ">nuS", &ar); // pops function
	if (ar.what[0] != 'L') {
		lua_CFunction cfunction = lua_tocfunction(L, -1);
		luaL_error(L, "attempt to persist a C function '%s'", ar.name);
		return;
	}

	buffer->write_byte(LS_FUNCTION);
	SerializeBuffer function_buffer;
	lua_pushvalue(L, idx); // push function
	lua_dump(L, lua_serialize_buffer_writer, (void*)&function_buffer);
	buffer->write_int(function_buffer.size());
	buffer->write_raw(function_buffer.data(), function_buffer.size());

	buffer->write_int(ar.nups);
	for (int i = 1; i <= ar.nups; i++) {
		const char* name = lua_getupvalue(L, -1, i);
		LCOMMON_ASSERT(name);
		encode(-1);
		lua_pop(L, 1);
	}

	lua_pop(L, 1); // pop function
}

bool LuaSerializeContext::test_has_metamethod(int idx) {
    idx = abs_index(L, idx);
	lua_pushvalue(L, idx);
	int old_top = lua_gettop(L);
	if (luaL_getmetafield(L, idx, "__save")) {
		buffer->write_byte(LS_REF_METAMETHOD);
		encode_metatable(-2);

		lua_pushvalue(L, -2);
		lua_pushvalue(L, this->context_object);
		lua_call(L, 2, LUA_MULTRET);
		// Encode all the returned values
		int stack_top = lua_gettop(L);
		int nret = stack_top - old_top;
		buffer->write_int(nret); // Save amount
		for (int i = stack_top - nret + 1; i <= stack_top; i++) {
			encode(i);
		}
		lua_settop(L, old_top - 1); // Restore stack
		store_ref_id(idx);
		return true;
	}
	lua_pop(L, 1);
	return false;
}

void LuaSerializeContext::encode_metatable(int idx) {
    idx = abs_index(L, idx);
	if (!lua_getmetatable(L, idx)) {
		lua_pushnil(L);
	}
	encode(-1); // Encode metatable
	lua_pop(L, 1);
}

void LuaSerializeContext::encode(int idx) {
    idx = abs_index(L, idx);
	int old_stack = lua_gettop(L);
	switch (lua_type(L, idx)) {
	case LUA_TNUMBER:
		buffer->write_byte(LS_NUMBER);
		buffer->write_double(lua_tonumber(L, idx));
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	case LUA_TBOOLEAN:
		buffer->write_byte(LS_BOOL);
		buffer->write_byte(lua_toboolean(L, idx));
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	case LUA_TNIL:
		buffer->write_byte(LS_NIL);
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	case LUA_TSTRING: {
		size_t len;
		const char* str = lua_tolstring(L, idx, &len);
		buffer->write_byte(LS_STRING);
		buffer->write_int(len);
		buffer->write_raw(str, len);
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	}
	case LUA_TTABLE:
		if (!test_has_ref(idx) && !test_has_metamethod(idx)) {
			store_ref_id(idx);
			buffer->write_byte(LS_TABLE);
			encode_metatable(idx);
			encode_table(idx);
		}
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	case LUA_TUSERDATA:
	case LUA_TLIGHTUSERDATA:
		if (!test_has_ref(idx) && !test_has_metamethod(idx)) {
			luaL_error(L, "attempt to encode userdata (no __save hook)");
		}
		return;
	case LUA_TFUNCTION:
		if (!test_has_ref(idx)) {
			store_ref_id(idx);
			encode_function(idx);
		}
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	default:
		luaL_error(L, "cannot persist object of type '%s'", lua_typename(L, lua_type(L, idx)));
		return;
	}
}

/***************************************************************************************
 *                           Lua value decoding                                        *
 ***************************************************************************************/

void LuaSerializeContext::decode() {
	_decode(buffer->read_byte());
}

void LuaSerializeContext::_decode(int type) {
	int old_stack = lua_gettop(L) + 1;
	switch (type) {
	case LS_NUMBER:
		lua_pushnumber(L, buffer->read_double());
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	case LS_BOOL:
		lua_pushboolean(L, buffer->read_byte());
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	case LS_STRING:
		decode_string();
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	case LS_FUNCTION:
		decode_function();
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	case LS_TABLE:
		lua_newtable(L);
		store_object(-1); // Store immediately to let references resolve
		decode(); // Decode metatable
		decode_table(-2); // Note: does not push table
		lua_setmetatable(L, -2);
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	case LS_REF_ID: {
        int recorded_type = buffer->read_byte();
		int id = buffer->read_int();
		lua_rawgeti(L, this->index_to_obj, id);
		if (lua_isnil(L, -1)) {
			lua_pop(L,1);
                        lua_newtable(L);//luaL_error(L, "LS_REF_ID resolved to nil loading id=%d (problem with serialized Lua value!)", id);
		} //else if (lua_type(L, -1) != recorded_type){
//	        const char* was = lua_typename(L, lua_type(L, -1));
//	        const char* expected = lua_typename(L, recorded_type);
//	        luaL_error(L, "Reference decoding for '%s' returned a value of wrong type. Got a '%s', expected a '%s'.", was, expected);
//	    }
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	}
	case LS_REF_NAME:
		decode_ref_name();
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	case LS_REF_METAMETHOD:
		decode_ref_metamethod();
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	case LS_NIL:
		lua_pushnil(L);
		LCOMMON_ASSERT(old_stack == lua_gettop(L));
		return;
	case LS_TABLE_END_SENTINEL:
		luaL_error(L, "Misplaced table end sentinel (problem with serialized Lua value!)");
		return;
	default:
		luaL_error(L, "Garbage object type %d encountered (problem with serialized Lua value!)", type);
		return;
	}
}

void LuaSerializeContext::decode_table(int idx) {
    idx = abs_index(L, idx);
	lua_pushvalue(L, idx);
	int type;
	while ((type = buffer->read_byte()) != LS_TABLE_END_SENTINEL) {
		_decode(type);
		if (lua_type(L, -1) == LUA_TSTRING) {
			//printf("DECODING '%s'\n", lua_tostring(L, -1));
		}
		decode();
		lua_rawset(L, -3);
	}
	lua_pop(L, 1);
    {LuaValue VALUE{L, idx};
        decode_keys(*buffer, VALUE);}
}

// Simple string holder to pass to our lua_Reader.
struct _Str {
	const char* str;
	size_t len;
	_Str(const char* str, size_t len) {
		this->str = str, this->len = len;
	}
};

static const char* lua_serialize_buffer_reader(lua_State *L, void* buffer, size_t* size) {
	_Str* s = (_Str*)buffer;
	const char* ret_str = s->str;
	*size = s->len;
	s->str = NULL, s->len = 0;
	return ret_str;
}

void LuaSerializeContext::decode_function() {
	SerializeBuffer function_buffer;
	int function_size = buffer->read_int();
	_Str str(buffer->fetch_raw(function_size), function_size);
	lua_load(L, lua_serialize_buffer_reader, &str, "<decoded function>");
	int func_id = lua_gettop(L);

	store_object(func_id);
        //printf("DECODING FUNC %d\n", next_decode_index);

	size_t nups = buffer->read_int();
	for (int i = 1; i <= nups; i++) {
		decode();
		const char* name = lua_setupvalue(L, func_id, i);
		if (!name) {
			luaL_error(L, "Unable to find upvalue, exitting...");
			return;
		}
	}
}

void LuaSerializeContext::decode_string() {
	size_t len = buffer->read_int();
	lua_pushlstring(L, buffer->fetch_raw(len), len);
}

void LuaSerializeContext::encode(const LuaValue& value) {
	if (!value.empty()) {
		value.push();
		encode(-1);
		lua_pop(L, 1);
	} else {
		buffer->write_byte(LS_NOVALUE);
	}
}

void LuaSerializeContext::decode(LuaValue& value) {
	int type = buffer->read_byte(); // is empty ?
	if (type != LS_NOVALUE) {
		_decode(type);
		if (value.empty()) {
			value.init(L);
		}
		value.pop();
	} else {
		value.clear();
	}
}

void LuaSerializeContext::decode_ref_name() {
    int recorded_type = buffer->read_byte();
	decode_string();
	lua_pushvalue(L, -1);
	lua_rawget(L, this->index_to_obj);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
//		printf("FAILED %s\n", lua_tostring(L, -1));
		// Handle failure:
		lua_pushvalue(L, this->index_failure_function);
		lua_pushvalue(L, this->context_object);
		lua_pushvalue(L, -3); // Push string
		lua_call(L, 2, 1);
	}

	if (lua_isnil(L, -1)) {
		const char* str = lua_tostring(L, -2);
		luaL_error(L, "Fallback function failed to return decoded value for '%s'.", str);
	} // else if (lua_type(L, -1) != recorded_type){
//	    const char* was = lua_typename(L, lua_type(L, -1));
//	    const char* expected = lua_typename(L, recorded_type);
//        luaL_error(L, "Reference decoding for '%s' returned a value of wrong type. Got a '%s', expected a '%s'.", was, expected);
//	}
//
	lua_replace(L, -2); // Replace string with decoded value

}

void LuaSerializeContext::store_object(int idx) {
	//printf("LOADING %d AS %s\n", (int)next_decode_index, lua_typename(L, lua_type(L, idx)));
	lua_pushvalue(L, idx);
	lua_rawseti(L, this->index_to_obj, next_decode_index++);
}

void LuaSerializeContext::decode_ref_metamethod() {
	int old_top = lua_gettop(L);
	decode(); // Decode metatable
	lua_pushliteral(L, "__load");
	lua_rawget(L, -2);

	lua_pushvalue(L, this->context_object);
	int nargs = buffer->read_int();
	for (int i = 0; i < nargs; i++) {
		decode();
	}

	lua_call(L, nargs + 1, 1);
	lua_replace(L, -2); // Remove metatable from stack
	store_object(-1);
}


void luaserialize_encode(lua_State* L, SerializeBuffer& serializer, int idx) {
	lua_pushvalue(L, idx);
	LuaSerializeContext context = LuaSerializeContext::push3(L, &serializer, 1, 1);
	context.encode(-2);
	lua_pop(L, 4);
}
void luaserialize_decode(lua_State* L, SerializeBuffer& serializer) {
	LuaSerializeContext context = LuaSerializeContext::push3(L, &serializer, 1, 1);
	context.decode();
	lua_replace(L, -3); // Replace first pushed value
	lua_pop(L, 2);
}

void luaserialize_encode(lua_State* L, SerializeBuffer& serializer, const LuaValue& value) {
	LuaSerializeContext context = LuaSerializeContext::push3(L, &serializer, 1, 1);
	context.encode(value);
	lua_pop(L, 3);
}
void luaserialize_decode(lua_State* L, SerializeBuffer& serializer, LuaValue& value) {
	LuaSerializeContext context = LuaSerializeContext::push3(L, &serializer, 1, 1);
	context.decode(value);
	lua_pop(L, 3);
}

LuaSerializeContext LuaSerializeConfig::push4(SerializeBuffer& serializer) {
	this->obj_to_index.push();
	this->index_to_obj.push();
	this->index_failure_function.push();
	this->context_object.push();
	return LuaSerializeContext::ref_top4(L, &serializer, this->next_encode_index, this->next_decode_index);
}

void LuaSerializeConfig::encode(SerializeBuffer& serializer, int idx) {
	lua_pushvalue(L, idx);
	int val = lua_gettop(L);
	LuaSerializeContext context = push4(serializer);
	context.encode(val);
	sync_indices(context);
	lua_pop(L, 5);
}

void LuaSerializeConfig::decode(SerializeBuffer& serializer) {
	int val = lua_gettop(L);
	LuaSerializeContext context = push4(serializer);
	context.decode();
	sync_indices(context);
	// Place decoded value before pushed values
	lua_replace(L, -5);
	lua_pop(L, 4);
}

void LuaSerializeConfig::encode(SerializeBuffer& serializer,
		const LuaValue& value) {
	LuaSerializeContext context = push4(serializer);
	context.encode(value);
	sync_indices(context);
	lua_pop(L, 4);
    encode_keys(serializer,  value);
}


void LuaSerializeConfig::decode(SerializeBuffer& serializer, LuaValue& value) {
	LuaSerializeContext context = push4(serializer);
	context.decode(value);
	sync_indices(context);
	lua_pop(L, 4);
    decode_keys(serializer, value);
}

static void lua_table_clear(lua_State* L, int table_idx) {
    int old_top = lua_gettop(L);
    lua_pushvalue(L, table_idx); // Push for setting into table
    table_idx = lua_gettop(L);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        lua_pushvalue(L, -2); // Get key
        lua_pushnil(L); // Push nil to delete the key with
        lua_rawset(L, table_idx);
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // Pop extra table copy.
    assert(old_top == lua_gettop(L));
    assert(lua_objlen(L, table_idx) == 0);
}

void LuaSerializeConfig::reset() {
     // Clear all our tables, while making sure the references held by
     // our lua state remain valid:
     obj_to_index.push();
     lua_table_clear(L, -1);
     lua_pop(L, 1);
     index_to_obj.push();
     lua_table_clear(L, -1);
     lua_pop(L, 1);
     context_object.push();
     lua_table_clear(L, -1);
     lua_pop(L, 1);
}
