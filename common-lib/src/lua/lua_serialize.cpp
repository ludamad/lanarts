/*
 * Adapted from lmarshal.c:
 * A Lua library for serializing and deserializing Lua values
 * Original license follows.
 */

/*
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

#include <stdlib.h>
#include <string.h>

#include <exception>
#include <lua.hpp>
#include <luawrap/luawrap.h>

#include "lua_serialize.h"
#include "SerializeBuffer.h"

static const unsigned int MAR_TREF = 1;
static const unsigned int MAR_TREF_STR = 2;
// Table without a meta-table
static const unsigned int MAR_TVAL = 3;
// Table with a meta-table
static const unsigned int MAR_TVAL_WITH_META = 4;
static const unsigned int MAR_TUSR = 5;

static const unsigned int MAR_CHR = 1;
static const unsigned int MAR_I64 = 8;

static const unsigned int SEEN_IDX = 3;

typedef unsigned int uint32_t;

//TODO: remove completely
typedef struct mar_Buffer {
	size_t size;
	size_t seek;
	size_t head;
	char* data;
} mar_Buffer;

static int mar_encode_table(lua_State *L, SerializeBuffer& buf, size_t *idx);
static int mar_decode_table(lua_State *L, const char* buf, size_t len,
		size_t *idx);

void mar_encode_value(lua_State *L, SerializeBuffer& buf, int val, size_t *idx);
void mar_decode_value(lua_State* L, const char *buf, size_t len, const char **p,
		size_t *idx);

static int lua_serialize_buffer_writer(lua_State* L, const void* data, size_t len, void* buffer) {
	((SerializeBuffer*)buffer)->write_raw((const char*) data, len);
	return 0;
}

static const char* buf_read(lua_State *L, mar_Buffer* buf, size_t *len) {
	if (buf->seek < buf->head) {
		buf->seek = buf->head;
		*len = buf->seek;
		return buf->data;
	}
	*len = 0;
	return NULL;
}

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

static void pretty_print(LuaField field) {
	lua_State* L = field.luastate();
	luawrap::globals(L)["pretty_print"].push();
	field.push();
	lua_call(L, 1, 0);
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

#define mar_incr_ptr(l) \
    if (((*p)-buf)+(l) > len) luaL_error(L, "bad code"); (*p) += (l);

#define mar_next_len(l,T) \
    if (((*p)-buf)+sizeof(T) > len) luaL_error(L, "bad code"); \
    l = *(T*)*p; (*p) += sizeof(T);

static void mar_handle_persist_closure(lua_State* L, const char *buf, size_t len, const char **p, size_t *idx) {
	size_t persistargs;
	mar_next_len(persistargs, uint32_t);
	for (int i = 0; i < persistargs; i++) {
		mar_decode_value(L, buf, len, p, idx);
	}
	// First argument is function, so persistargs-1 arguments are passed
	lua_call(L, persistargs - 1, 1);
//	lua_pushvalue(L, -1);
//	lua_rawseti(L, SEEN_IDX, (*idx)++);
}

void lua_store_submodule_refs(LuaField submodule, const char* name, LuaField str2obj_table, LuaField obj2str_table, bool recurse = true);

static bool object_was_marked_mutable(LuaField value) {
	lua_State* L = value.luastate();
	luawrap::registry(L)["serialize_always"].push();
	value.push();
	lua_rawget(L, -2);
	bool wasnil = lua_isnil(L, -1);
	lua_pop(L, 2); // table & value
	return !wasnil;
}

static void store_object(LuaField key, LuaField value, LuaField str2obj_table, LuaField obj2str_table, bool recurse = true) {
	lua_State* L = key.luastate();
	int ntop = lua_gettop(L);

	if (object_was_marked_mutable(value)) {
//		printf("Object %s was marked mutable and will not be treated as a constant.\n", key.to_str());
		return;
	}

	// Store the string->object pair
	str2obj_table.push();
	key.push();// Push string
	value.push();// Push value
	lua_rawset(L, -3);
	lua_pop(L, 1);

	// Check if the object->string pair already has the object,
	// We should avoid over-writing it
	obj2str_table.push();
	value.push();// Push value
	lua_rawget(L, -2);

	// The object has already been stored
	if (!lua_isnil(L, -1)) {
		lua_settop(L, ntop);
		return;
	}
	lua_pop(L, 2); // pop table & result

	obj2str_table.push();
	value.push();// Push value
	key.push();// Push string
//	printf("%s: 0x%lX, type=%s\n",lua_tostring(L, -1), lua_topointer(L, -2), lua_typename(L, lua_type(L, -2)));
	lua_rawset(L, -3);
	lua_pop(L, 1);

	value.push();
	if (lua_istable(L, -1) && recurse) {
		lua_store_submodule_refs(value, key.to_str(), str2obj_table, obj2str_table);
	}

	lua_settop(L, ntop);
}

void lua_register_serialization_constant(const char* key, LuaField object) {
	lua_State* L = object.luastate();
	LuaField table = luawrap::ensure_table(luawrap::registry(L)["serialize_never"]);
	table[key] = object;
}

void lua_register_serialization_mutable(LuaField object) {
	lua_State* L = object.luastate();
	LuaField table = luawrap::ensure_table(luawrap::registry(L)["serialize_always"]);
	table.push();
	object.push();
	lua_pushboolean(L, true); // Arbitrary non-nil value
	lua_rawset(L, -3);
	lua_pop(L, 1); // Table
}

// TODO: Less hackish
void lua_store_submodule_refs(LuaField submodule, const char* name, LuaField str2obj_table, LuaField obj2str_table, bool recurse) {
	lua_State* L = submodule.luastate();

	// Iterate the submodule
	submodule.push();
	if (lua_getmetatable(L, -1)) {
		int type = lua_type(L, -1);
		lua_pushfstring(L, "%s;;<meta>", name);
		if (type != LUA_TSTRING && type != LUA_TNUMBER && type != LUA_TBOOLEAN && type != LUA_TBOOLEAN) {
			store_object(LuaStackValue(L, -1), LuaStackValue(L, -2), str2obj_table, obj2str_table, recurse);
		}
		lua_pop(L, 2);
	}
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		if (lua_type(L, -2) == LUA_TSTRING) {
			int val_idx = lua_gettop(L);
			lua_pushfstring(L, "%s;%s", name, lua_tostring(L, -2));
			int type = lua_type(L, val_idx);
			if (type != LUA_TSTRING && type != LUA_TNUMBER && type != LUA_TBOOLEAN && type != LUA_TBOOLEAN) {
				store_object(LuaStackValue(L, -1), LuaStackValue(L, val_idx), str2obj_table, obj2str_table, recurse);
			}
			lua_pop(L, 2);
		} else {
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);
}

// TODO: Less hackish
static void decode_ref_miss(lua_State* L, const char* ref_str, size_t len) {
	int i;
	for (i = 0; i < len; i++) {
		if (ref_str[i] == ';') {
			break;
		}
	}
	lua_pushlstring(L, ref_str, i);
//	printf("Got ref miss with '%s', importing '%s'\n", ref_str, lua_tostring(L, -1));
	luawrap::globals(L)["import"].push();
	lua_pushvalue(L, -2);
	lua_call(L, 1, 1);
	store_object(LuaStackValue(L, -2), LuaStackValue(L, -1),  luawrap::registry(L)["serialize_key2obj"], luawrap::registry(L)["serialize_obj2key"]);
	lua_pop(L, 2);
}

static bool decode_ref(lua_State* L, const char *buf, size_t len, const char **p, int tag) {
	if (tag == MAR_TREF) {
		int ref;
		mar_next_len(ref, int);
		lua_rawgeti(L, SEEN_IDX, ref);
		return true;
	} else if (tag == MAR_TREF_STR) {
		size_t l;
		mar_next_len(l, uint32_t);
		lua_pushlstring(L, *p, l);
		mar_incr_ptr(l);

		lua_pushvalue(L, -1); // duplicate string
		lua_rawget(L, SEEN_IDX);
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			decode_ref_miss(L, lua_tostring(L, -1), l);
			lua_pushvalue(L, -1); // duplicate string

			lua_rawget(L, SEEN_IDX);
			if (lua_isnil(L, -1)) {
				printf("'%s'\n", lua_tostring(L,-2));
				throw std::runtime_error(format("Could not find matching value for key '%s' in deserialization.", lua_tostring(L, -2)));
			}
		}
		lua_replace(L, -2);

		return true;
	}
	return false;
}

void mar_decode_value(lua_State* L, const char *buf, size_t len, const char **p,
		size_t *idx) {
	int old_stack_top = lua_gettop(L);
	size_t l;
	char val_type = **p;
	mar_incr_ptr(MAR_CHR);
	switch (val_type) {
	case LUA_TBOOLEAN:
		lua_pushboolean(L, *(char*)*p);
		mar_incr_ptr(MAR_CHR);
		break;
	case LUA_TNUMBER:
		lua_pushnumber(L, *(lua_Number*)*p);
		mar_incr_ptr(MAR_I64);
		break;
	case LUA_TSTRING:
		mar_next_len(l, uint32_t);
		lua_pushlstring(L, *p, l);
		mar_incr_ptr(l);
		break;
	case LUA_TTABLE: {
		char tag = *(char*)*p;
		mar_incr_ptr(MAR_CHR);
		if (decode_ref(L, buf, len, p, tag)) {
		} else if (tag == MAR_TVAL_WITH_META || tag == MAR_TVAL) {
			lua_newtable(L);
			lua_pushvalue(L, -1);
//			printf("Decoding type %d = %s\n", *idx, lua_typename(L, lua_type(L, -1)));
			lua_rawseti(L, SEEN_IDX, (*idx)++);
			if (tag == MAR_TVAL_WITH_META) {
				mar_decode_value(L, buf, len, p, idx);
//				printf("Object has metatable %lX\n", lua_topointer(L, -1));
//				printf("Pushed metatable, type=%s\n", lua_typename(L, lua_type(L, -1)));
				lua_pushvalue(L, -2); // Push for decoding
			}
			mar_next_len(l, uint32_t);
			mar_decode_table(L, *p, l, idx);
			mar_incr_ptr(l);
			if (tag == MAR_TVAL_WITH_META) {
				lua_pop(L, 1);
				lua_setmetatable(L, -2);
			}
		} else if (tag == MAR_TUSR) {
			mar_handle_persist_closure(L, buf, len, p, idx);
		} else {
			luaL_error(L, "bad encoded data");
		}
		break;
	}
	case LUA_TFUNCTION: {
		size_t nups;
		int i;
		mar_Buffer dec_buf;
		char tag = *(char*)*p;
		mar_incr_ptr(1);
		if (decode_ref(L, buf, len, p, tag)) {
					// done
		} else {
			mar_next_len(l, uint32_t);
			dec_buf.data = (char*)*p;
			dec_buf.size = l;
			dec_buf.head = l;
			dec_buf.seek = 0;
			lua_load(L, (lua_Reader)buf_read, &dec_buf, "=marshal");
			mar_incr_ptr(l);

			lua_pushvalue(L, -1);
			lua_rawseti(L, SEEN_IDX, (*idx)++);
//			printf("Decoding type %d = %s\n", *idx -1, lua_typename(L, lua_type(L, -1)));

			mar_next_len(l, uint32_t);
			lua_newtable(L);
			mar_decode_table(L, *p, l, idx);
			nups = lua_objlen(L, -1);
			for (i = 1; i <= nups; i++) {
				lua_rawgeti(L, -1, i);
				lua_setupvalue(L, -3, i);
			}
			lua_pop(L, 1);
			mar_incr_ptr(l);
		}
		break;
	}
	case LUA_TUSERDATA: {
		char tag = *(char*)*p;
		mar_incr_ptr(MAR_CHR);
		if (decode_ref(L, buf, len, p, tag)) {
			// done
		} else if (tag == MAR_TUSR) {
			mar_handle_persist_closure(L, buf, len, p, idx);
		} else { /* tag == MAR_TVAL */
			lua_pushnil(L);
		}
		break;
	}
	case LUA_TNIL:
	case LUA_TTHREAD:
		lua_pushnil(L);
		break;
	default:
		luaL_error(L, "bad code");
		break;
	}
	fflush(stdout);
	LCOMMON_ASSERT(old_stack_top + 1 == lua_gettop(L));
}

#define MAR_LOG_DECODE

#ifdef MAR_LOG_DECODE
int tabs = 0;
#endif

static int mar_decode_table(lua_State *L, const char* buf, size_t len,
		size_t *idx) {
	const char* p;
	p = buf;
#ifdef MAR_LOG_DECODE
	tabs++;
#endif
	while (p - buf < len) {
		mar_decode_value(L, buf, len, &p, idx);
#ifdef MAR_LOG_DECODE
		if (lua_type(L, -1) == LUA_TSTRING) {
			for (int t = 0; t < tabs; t++) {
				putchar('\t');
			}
			printf("Decoding '%s'\n", lua_tostring(L, -1));
		}
#endif
		mar_decode_value(L, buf, len, &p, idx);
		lua_settable(L, -3);
	}
#ifdef MAR_LOG_DECODE
	tabs--;
#endif
	return 1;
}

static SerializeBuffer* __cheathack_global;

int mar_encode(lua_State* L) {
	if (lua_isnone(L, 1)) {
		lua_pushnil(L);
	}
	while (lua_gettop(L) < 3) {
		lua_newtable(L);
	}
	for (int i = 2; i <= 3; i++) {
		if (lua_isnil(L, i)) {
			lua_newtable(L);
			lua_replace(L, i);
		} else if (!lua_istable(L, i)) {
			luaL_error(L, "bad argument #%d to encode (expected table)", i);
		}
	}

	size_t idx = luawrap::defaulted(luawrap::registry(L)["serialize_idx"], 1);
	size_t len = lua_objlen(L, 2);
	size_t goal = idx + len - 1;
	for (; idx <= goal; idx++) {
		lua_rawgeti(L, 2, idx);
		lua_pushinteger(L, idx);
		lua_rawset(L, SEEN_IDX);
	}

	lua_pushvalue(L, 1);
	mar_encode_value(L, *__cheathack_global, -1, &idx);
	//remove seen table and value copy
	lua_pop(L, 2);
	luawrap::registry(L)["serialize_idx"] = idx;

	return 0;
}

int mar_decode(lua_State* L) {
	if (lua_isnone(L, 1)) {
		lua_pushnil(L);
	}
	while (lua_gettop(L) < 3) {
		lua_newtable(L);
	}
	for (int i = 2; i <= 3; i++) {
		if (lua_isnil(L, i)) {
			lua_newtable(L);
			lua_replace(L, i);
		} else if (!lua_istable(L, i)) {
			luaL_error(L, "bad argument #%d to decode (expected table)", i);
		}
	}

	size_t len = lua_objlen(L, 2), start_len = lua_objlen(L, 3);
	size_t idx;
	for (idx = start_len + 1; idx <= start_len + len; idx++) {
		lua_rawgeti(L, 2, idx);
		lua_rawseti(L, SEEN_IDX, idx);
	}

	const char* buf_data = __cheathack_global->data() + __cheathack_global->read_position();
	const char* data_iter = buf_data;
	size_t buf_size = __cheathack_global->size() - __cheathack_global->read_position();
	mar_decode_value(L, buf_data, buf_size, &data_iter, &idx);

	__cheathack_global->move_read_position(data_iter - buf_data);
	return 1;
}

int mar_clone(lua_State* L) {
	mar_encode(L);
	lua_replace(L, 1);
	mar_decode(L);
	return 1;
}

static void ensure_serialization_state(lua_State* L) {
	if (!luawrap::registry(L)["serialize_key2obj"].isnil()) {
		return;
	}
	luawrap::ensure_table(luawrap::registry(L)["serialize_never"]);
	luawrap::ensure_table(luawrap::registry(L)["serialize_always"]);

	luawrap::registry(L)["serialize_key2obj"].newtable();
	luawrap::registry(L)["serialize_obj2key"].newtable();

	// Serialize globals
	lua_pushstring(L, "_G");
	store_object(LuaStackValue(L, -1), luawrap::globals(L),
			luawrap::registry(L)["serialize_key2obj"],
			luawrap::registry(L)["serialize_obj2key"], false);
	lua_pop(L, 1); // pop "_G"

	luawrap::ensure_table(luawrap::globals(L)["_IMPORTED"]).push();
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		LuaStackValue(L, -1)[1].push(); // unbox
		if (!lua_isnil(L, -1)) {
			store_object(LuaStackValue(L, -3), LuaStackValue(L, -1),
					luawrap::registry(L)["serialize_key2obj"],
					luawrap::registry(L)["serialize_obj2key"]);
		}
		lua_pop(L, 2);
	}
	lua_pop(L, 1);

	luawrap::registry(L)["serialize_never"].push();
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		return;
	}
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		store_object(LuaStackValue(L, -2), LuaStackValue(L, -1),
				luawrap::registry(L)["serialize_key2obj"],
				luawrap::registry(L)["serialize_obj2key"]);
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
}

/* lcommon adapter code starts here */
void lua_serialize(SerializeBuffer& serialize, lua_State* L, int nargs) {
	__cheathack_global = &serialize;

	lua_pushcfunction(L, mar_encode);
	lua_insert(L, -nargs-1);
	lua_call(L, nargs, 0);
}

void lua_deserialize(SerializeBuffer& serialize, lua_State* L, int nargs) {
	__cheathack_global = &serialize;

	lua_pushcfunction(L, mar_decode);
	lua_insert(L, -nargs-1);
	lua_call(L, nargs, 1);
}

#include <luawrap/LuaValue.h>

void lua_serialize(SerializeBuffer& serializer, lua_State* L,
		const LuaValue& value) {
	ensure_serialization_state(L);

	serializer.write_byte(value.empty());

	if (!value.empty()) {
		value.push();
		lua_newtable(L);
		luawrap::registry(L)["serialize_obj2key"].push();

		lua_serialize(serializer, L, 3);
	}
}

void lua_clear_serialization_state(lua_State* L) {
	lua_pushnil(L);
	luawrap::registry(L)["serialize_obj2key"].pop();
	lua_pushnil(L);
	luawrap::registry(L)["serialize_key2obj"].pop();
	lua_pushnil(L);
	luawrap::registry(L)["serialize_idx"].pop();
}

void lua_deserialize(SerializeBuffer& serializer, lua_State* L,
		LuaValue& value) {
	ensure_serialization_state(L);

	bool isnull;
	serializer.read_byte(isnull);

	if (!isnull) {
		lua_pushnil(L);
		lua_newtable(L);
		luawrap::registry(L)["serialize_key2obj"].push();
		lua_deserialize(serializer, L, 3);

		value.init(L);
		value.pop();
	} else {
		value.clear();
	}
}
