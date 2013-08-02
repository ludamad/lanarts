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

#include <lua.hpp>
#include <luawrap/luawrap.h>

#include "lua_serialize.h"
#include "SerializeBuffer.h"

static const unsigned int MAR_TREF = 1;
// Table without a meta-table
static const unsigned int MAR_TVAL = 2;
// Table with a meta-table
static const unsigned int MAR_TVAL_WITH_META = 3;
static const unsigned int MAR_TUSR = 4;

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
	lua_rawget(L, table_idx);
	bool was_seen = !lua_isnil(L, -1);
	if (was_seen) {
		buf.write_byte(MAR_TREF);
		buf.write_int(lua_tointeger(L, -1));
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
		if (!lua_isfunction(L, -1)) {
			luaL_error(L, "__persist must return a function");
		}
		// Save the amount of returned values
		buf.write_int(lua_gettop(L) - stacksize);
		// Encode all the returned values
		for (int i = stacksize + 1; i <= lua_gettop(L); i++) {
			mar_encode_value(L, buf, i, idx);
		}
		lua_settop(L, stacksize);
	}
	return false;
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
		lua_pushvalue(L, -1);
		if (!buf_write_if_seen(L, buf, SEEN_IDX)
				&& !buf_try_persist_hook(L, buf, idx)) {
			lua_pushvalue(L, -1);
			lua_pushinteger(L, (*idx)++);
			lua_rawset(L, SEEN_IDX);

			if (lua_getmetatable(L, -1)) {
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

			lua_pushvalue(L, -1);
			lua_getinfo(L, ">nuS", &ar);
			if (ar.what[0] != 'L') {
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
	lua_pushvalue(L, -1);
	lua_rawseti(L, SEEN_IDX, (*idx)++);
}

void mar_decode_value(lua_State* L, const char *buf, size_t len, const char **p,
		size_t *idx) {
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
		if (tag == MAR_TREF) {
			int ref;
			mar_next_len(ref, int);
			lua_rawgeti(L, SEEN_IDX, ref);
		} else if (tag == MAR_TVAL_WITH_META || tag == MAR_TVAL) {
			if (tag == MAR_TVAL_WITH_META) {
				mar_decode_value(L, buf, len, p, idx);
			}
			mar_next_len(l, uint32_t);
			lua_newtable(L);
			lua_pushvalue(L, -1);
			lua_rawseti(L, SEEN_IDX, (*idx)++);
			mar_decode_table(L, *p, l, idx);
			mar_incr_ptr(l);
			if (tag == MAR_TVAL_WITH_META) {
				// Rearrange with meta-table (decoded above)
				lua_insert(L, -2);
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
		if (tag == MAR_TREF) {
			int ref;
			mar_next_len(ref, int);
			lua_rawgeti(L, SEEN_IDX, ref);
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
		if (tag == MAR_TREF) {
			int ref;
			mar_next_len(ref, int);
			lua_rawgeti(L, SEEN_IDX, ref);
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
}

static int mar_decode_table(lua_State *L, const char* buf, size_t len,
		size_t *idx) {
	const char* p;
	p = buf;
	while (p - buf < len) {
		mar_decode_value(L, buf, len, &p, idx);
		mar_decode_value(L, buf, len, &p, idx);
		lua_settable(L, -3);
	}
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

	size_t len = lua_objlen(L, 2), idx;
	lua_newtable(L);
	for (idx = 1; idx <= len; idx++) {
		lua_rawgeti(L, 2, idx);
		lua_pushinteger(L, idx);
		lua_rawset(L, SEEN_IDX);
	}

	lua_pushvalue(L, 1);
	mar_encode_value(L, *__cheathack_global, -1, &idx);
	//remove seen table and value copy
	lua_pop(L, 2);

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

	size_t len = lua_objlen(L, 1);
	size_t idx;
	for (idx = 1; idx <= len; idx++) {
		lua_rawgeti(L, 1, idx);
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
	serializer.write_byte(value.empty());

	if (!value.empty()) {
		value.push();
		lua_serialize(serializer, L, 1);
	}
}

void lua_deserialize(SerializeBuffer& serializer, lua_State* L,
		LuaValue& value) {
	bool isnull;
	serializer.read_byte(isnull);

	if (!isnull) {
		value.init(L);
		lua_deserialize(serializer, L, 1);
		value.pop();
	} else {
		value.clear();
	}
}

