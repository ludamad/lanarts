/* Loosely based off lmarshal.c. */

#include <stdlib.h>
#include <string.h>

#include <exception>
#include <lua.hpp>
#include <luawrap/luawrap.h>

#include "lua_serialize.h"
#include "SerializeBuffer.h"

static inline bool buf_write_if_seen(lua_State* L, SerializeBuffer& buf, int table_idx) {
//	printf("0x%lX %s\n", lua_topointer(L, -1), lua_typename(L, lua_type(L, -1)));
	lua_pushvalue(L, -1);
	lua_rawget(L, table_idx);
	bool was_seen = !lua_isnil(L, -1);
//	printf("0x%lX %s\n", lua_topointer(L, -2), was_seen ? "was seen" : "wasn't seen");
	if (was_seen) {
		if (lua_isnumber(L, -1)) {
			buf.write_byte(MAR_TREF);
			buf.write_int(lua_tointeger(L, -1));
//			printf("Writing object with integer ID\n");
		} else {
			size_t str_len;
			const char* str = lua_tolstring(L, -1, &str_len);
			printf("Test: '%s', %d chars\n", str, (int)str_len);
			buf.write_byte(MAR_TREF_STR);
			buf.write_int(str_len);
			buf.write_raw(str, str_len);
		}
		lua_pop(L, 1); // pop fully if was seen
	}
	lua_pop(L, 1);
	return was_seen;
}
static inline void buf_append(SerializeBuffer& buf, SerializeBuffer& sub_buf) {
	buf.write_int(sub_buf.size());
	buf.write_raw(sub_buf.data(), sub_buf.size());
}

static inline bool buf_try_persist_hook(lua_State *L, SerializeBuffer& buf, size_t *idx) {
	int stacksize = lua_gettop(L);
	if (luaL_getmetafield(L, -1, "__persist")) {
		buf.write_byte(MAR_TUSR);

		lua_pushvalue(L, -2); /* self */
		lua_call(L, 1, LUA_MULTRET);
		if (!lua_isfunction(L, stacksize+1)) {
			luaL_error(L, "__persist must return a function");
		}
		// Save the amount of returned values
		buf.write_int(lua_gettop(L) - stacksize);
		// Encode all the returned values
		int stack_top = lua_gettop(L);
		for (int i = stacksize + 1; i <= stack_top; i++) {
			mar_encode_value(L, buf, i, idx);
		}
		lua_settop(L, stacksize);
//		lua_pushvalue(L, -1);
//		lua_pushinteger(L, (*idx)++);
//		lua_rawset(L, SEEN_IDX);
		lua_pop(L, 1); // pop value
		return true;
	}
	return false;
}

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
	// Decodes by calling __unpersist on the object's decoded metatable, with decoded arguments:
	LS_PERSIST_HOOK
};

// On success, writes [LS_TABLE, (key encoding, value encoding)*, LS_TABLE_END_SENTINEL]
void LuaSerializeContext::encode_table(int idx) {
	buffer->write_byte(LS_TABLE);
	lua_pushvalue(L, idx);
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		encode(-2);
		encode(-1);
		lua_pop(L, 1);
	}
	buffer->write_byte(LS_TABLE_END_SENTINEL);
	lua_pop(L, 2);
}

// On success, writes [LS_REF_NAME, <ref name>]
bool LuaSerializeContext::test_has_reference(int idx) {
//	printf("0x%lX %s\n", lua_topointer(L, -1), lua_typename(L, lua_type(L, -1)));
	lua_pushvalue(L, idx);
	lua_rawget(L, this->obj_to_index);
	bool was_seen = !lua_isnil(L, -1);
//	printf("0x%lX %s\n", lua_topointer(L, -2), was_seen ? "was seen" : "wasn't seen");
	if (was_seen) {
		if (lua_isnumber(L, -1)) {
			buffer->write_byte(LS_REF_ID);
			buffer->write_int(lua_tointeger(L, -1));
			printf("Writing object with integer ID\n");
		} else {
			size_t str_len;
			const char* str = lua_tolstring(L, -1, &str_len);
			printf("Test: '%s', %d chars\n", str, (int)str_len);
			buffer->write_byte(LS_REF_NAME);
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

void LuaSerializeContext::store_reference(int idx) {
	lua_pushvalue(L, idx);
	lua_pushinteger(L, next_index++);
	lua_rawset(L, this->obj_to_index);
}

// Writes [LS_FUNCTION, <number of upvalues>, <upvalue encodings>]
void LuaSerializeContext::encode_function(int idx) {
	lua_pushvalue(L, idx);

	lua_Debug ar;
	lua_getinfo(L, ">nuS", &ar);
	if (ar.what[0] != 'L') {
		lua_CFunction cfunction = lua_tocfunction(L, -1);
		luaL_error(L, "attempt to persist a C function '%s'", ar.name);
		return;
	}

	store_reference(idx);

	buffer->write_byte(LS_FUNCTION);
	lua_dump(L, lua_serialize_buffer_writer, (void*)buffer);

	lua_pop(L, 1);

	buffer->write_int(ar.nups);
	for (int i = 1; i <= ar.nups; i++) {
		lua_getupvalue(L, -1, i);
		encode(-1);
		lua_pop(L, 1);
	}

	lua_pop(L, 1);
}

bool LuaSerializeContext::test_has_metamethod(int idx) {
	int old_top = lua_gettop(L);
	if (luaL_getmetafield(L, idx, "__persist")) {
		buffer->write_byte(LS_PERSIST_HOOK);

		store_reference(idx);

		lua_pushvalue(L, idx);
		lua_pushvalue(L, this->context_object);
		lua_call(L, 1, LUA_MULTRET);
		if (!lua_isfunction(L, old_top + 1)) {
			luaL_error(L, "__persist must return a function");
			return false;
		}
		// Encode all the returned values
		int stack_top = lua_gettop(L);
		int nret = stack_top - old_top;
		buffer->write_int(nret); // Save amount
		for (int i = stack_top - nret; i <= stack_top; i++) {
			encode(i);
		}
		lua_settop(L, old_top);
		return true;
	}
	return false;
}

void LuaSerializeContext::encode(int idx) {
	switch (lua_type(L, idx)) {
	case LUA_TNUMBER:
		buffer->write_byte(LS_NUMBER);
		buffer->write_byte(lua_toboolean(L, idx));
		return;
	case LUA_TBOOLEAN:
		buffer->write_byte(LS_BOOL);
		buffer->write_byte(lua_toboolean(L, idx));
		return;
	case LUA_TNIL:
		buffer->write_byte(LS_NIL);
		return;
	case LUA_TSTRING:
		encode_string(idx);
		return;
	case LUA_TTABLE:
		if (!test_has_reference(idx) && !test_has_metamethod(idx)) {
			encode_table(idx);
		}
		return;
	case LUA_TUSERDATA:
	case LUA_TLIGHTUSERDATA:
		if (!test_has_reference(idx) && !test_has_metamethod(idx)) {
			luaL_error(L, "attempt to encode userdata (no __persist hook)");
		}
		return;
	case LUA_TFUNCTION:
		if (!test_has_reference(idx)) {
			encode_function(idx);
		}
		return;
	}
}

void mar_encode_value(lua_State *L, SerializeBuffer& buf, int val, size_t *idx) {
	int val_type = lua_type(L, val);
	lua_pushvalue(L, val);

	buf.write_byte(val_type);
	switch (val_type) {
	case LUA_TBOOLEAN: {
		buf.write_byte(lua_toboolean(L, -1));
		break;
	}
	case LUA_TSTRING: {
		size_t len;
		const char* str_val = lua_tolstring(L, -1, &len);
		buf.write_int(len);
		buf.write_raw(str_val, len);
		break;
	}
	case LUA_TNUMBER: {
		buf.write(lua_tonumber(L, -1));
		break;
	}
	case LUA_TTABLE: {
//		pretty_print(LuaStackValue(L, -1));
		lua_pushvalue(L, -1);
		if (!buf_write_if_seen(L, buf, SEEN_IDX)
				&& !buf_try_persist_hook(L, buf, idx)) {
//			printf("Serializing type %d = %s\n", *idx, lua_typename(L, lua_type(L, -1)));
			lua_pushinteger(L, (*idx)++);
			lua_rawset(L, SEEN_IDX);

			if (lua_getmetatable(L, -1)) {
//				printf("Object has metatable %lX\n", lua_topointer(L, -1));
				// Table with a meta-table
				buf.write_byte(MAR_TVAL_WITH_META);
				mar_encode_value(L, buf, -1, idx);
				lua_pop(L, 1);
			} else {
				// Table without a meta-table
				buf.write_byte(MAR_TVAL);
			}

			SerializeBuffer rec_buf;
			lua_pushvalue(L, -1);
			mar_encode_table(L, rec_buf, idx);
			lua_pop(L, 1);

			buf_append(buf, rec_buf);
		}
		break;
	}
	case LUA_TFUNCTION: {
		lua_pushvalue(L, -1);
		if (!buf_write_if_seen(L, buf, SEEN_IDX)) {
			int i;
			lua_Debug ar;

			lua_getinfo(L, ">nuS", &ar);
			if (ar.what[0] != 'L') {
				lua_CFunction cfunction = lua_tocfunction(L, -1);
				luaL_error(L, "attempt to persist a C function '%s'", ar.name);
			}
			lua_pushvalue(L, -1);
			lua_pushinteger(L, (*idx)++);
			lua_rawset(L, SEEN_IDX);

			lua_pushvalue(L, -1);

			SerializeBuffer rec_buf;
			lua_dump(L, lua_serialize_buffer_writer, &rec_buf);
			buf.write_byte(MAR_TVAL);
			buf.write_int(rec_buf.size());
			buf.write_raw(rec_buf.data(), rec_buf.size());

			lua_pop(L, 1);

			lua_newtable(L);
			for (i = 1; i <= ar.nups; i++) {
				lua_getupvalue(L, -2, i);
				lua_rawseti(L, -2, i);
			}

			rec_buf.clear();
			mar_encode_table(L, rec_buf, idx);
			buf_append(buf, rec_buf);

			lua_pop(L, 1);
		}

		break;
	}
	case LUA_TUSERDATA: {
		int tag, ref;
		lua_pushvalue(L, -1);
		if (!buf_write_if_seen(L, buf, SEEN_IDX)
				&& !buf_try_persist_hook(L, buf, idx)) {
			luaL_error(L, "attempt to encode userdata (no __persist hook)");
			lua_pop(L, 1);
		}
		break;
	}
	case LUA_TNIL:
		break;
	default:
		luaL_error(L, "invalid value type (%s)", lua_typename(L, val_type));
		break;
	}
	lua_pop(L, 1);
}

static int mar_encode_table(lua_State *L, SerializeBuffer& buf, size_t *idx) {
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		mar_encode_value(L, buf, -2, idx);
		mar_encode_value(L, buf, -1, idx);
		lua_pop(L, 1);
	}
	return 1;
}

void lua_serialize(SerializeBuffer& serializer, lua_State* L,
		const LuaField& obj_to_index,
		const LuaField& index_to_obj,
		const LuaField& index_failure_function) {

}
