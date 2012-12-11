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

#include "lua_serialize.h"
#include "../SerializeBuffer.h"

#define UINT32_MAX 4294967296ul

#define MAR_TREF 1
#define MAR_TVAL 2
#define MAR_TUSR 3

#define MAR_CHR 1
#define MAR_I32 4
#define MAR_I64 8

#define MAR_MAGIC 0x8e
#define SEEN_IDX  3

typedef unsigned int uint32_t;

typedef struct mar_Buffer {
	size_t size;
	size_t seek;
	size_t head;
	char* data;
} mar_Buffer;

static int mar_encode_table(lua_State *L, mar_Buffer *buf, size_t *idx);
static int mar_decode_table(lua_State *L, const char* buf, size_t len,
		size_t *idx);

static void buf_init(lua_State *L, mar_Buffer *buf) {
	buf->size = 128;
	buf->seek = 0;
	buf->head = 0;
	if (!(buf->data = (char*)malloc(buf->size)))
		luaL_error(L, "Out of memory!");
}

static void buf_done(lua_State* L, mar_Buffer *buf) {
	free(buf->data);
}

static int buf_write(lua_State* L, void* str, size_t len, mar_Buffer *buf) {
	if (len > UINT32_MAX)
		luaL_error(L, "buffer too long");
	if (buf->size - buf->head < len) {
		size_t new_size = buf->size << 1;
		size_t cur_head = buf->head;
		while (new_size - cur_head <= len) {
			new_size = new_size << 1;
		}
		if (!(buf->data = (char*)realloc(buf->data, new_size))) {
			luaL_error(L, "Out of memory!");
		}
		buf->size = new_size;
	}
	memcpy(&buf->data[buf->head], str, len);
	buf->head += len;
	return 0;
}

static const char* buf_read(lua_State *L, mar_Buffer *buf, size_t *len) {
	if (buf->seek < buf->head) {
		buf->seek = buf->head;
		*len = buf->seek;
		return buf->data;
	}
	*len = 0;
	return NULL;
}

void mar_encode_value(lua_State *L, mar_Buffer *buf, int val, size_t *idx) {
	size_t l;
	int val_type = lua_type(L, val);
	lua_pushvalue(L, val);

	buf_write(L, (void*)&val_type, MAR_CHR, buf);
	switch (val_type) {
	case LUA_TBOOLEAN: {
		int int_val = lua_toboolean(L, -1);
		buf_write(L, (void*)&int_val, MAR_CHR, buf);
		break;
	}
	case LUA_TSTRING: {
		const char *str_val = lua_tolstring(L, -1, &l);
		buf_write(L, (void*)&l, MAR_I32, buf);
		buf_write(L, (void*)str_val, l, buf);
		break;
	}
	case LUA_TNUMBER: {
		lua_Number num_val = lua_tonumber(L, -1);
		buf_write(L, (void*)&num_val, MAR_I64, buf);
		break;
	}
	case LUA_TTABLE: {
		int tag, ref;
		lua_pushvalue(L, -1);
		lua_rawget(L, SEEN_IDX);
		if (!lua_isnil(L, -1)) {
			ref = lua_tointeger(L, -1);
			tag = MAR_TREF;
			buf_write(L, (void*)&tag, MAR_CHR, buf);
			buf_write(L, (void*)&ref, MAR_I32, buf);
			lua_pop(L, 1);
		} else {
			mar_Buffer rec_buf;
			lua_pop(L, 1);
			/* pop nil */
			if (luaL_getmetafield(L, -1, "__persist")) {
				tag = MAR_TUSR;

				lua_pushvalue(L, -2); /* self */
				lua_call(L, 1, 1);
				if (!lua_isfunction(L, -1)) {
					luaL_error(L, "__persist must return a function");
				}

				lua_remove(L, -2); /* __persist */

				lua_newtable(L);
				lua_pushvalue(L, -2); /* callback */
				lua_rawseti(L, -2, 1);

				buf_init(L, &rec_buf);
				mar_encode_table(L, &rec_buf, idx);

				buf_write(L, (void*)&tag, MAR_CHR, buf);
				buf_write(L, (void*)&rec_buf.head, MAR_I32, buf);
				buf_write(L, rec_buf.data, rec_buf.head, buf);
				buf_done(L, &rec_buf);
				lua_pop(L, 1);
			} else {
				tag = MAR_TVAL;

				lua_pushvalue(L, -1);
				lua_pushinteger(L, (*idx)++);
				lua_rawset(L, SEEN_IDX);

				lua_pushvalue(L, -1);
				buf_init(L, &rec_buf);
				mar_encode_table(L, &rec_buf, idx);
				lua_pop(L, 1);

				buf_write(L, (void*)&tag, MAR_CHR, buf);
				buf_write(L, (void*)&rec_buf.head, MAR_I32, buf);
				buf_write(L, rec_buf.data, rec_buf.head, buf);
				buf_done(L, &rec_buf);
			}
		}
		break;
	}
	case LUA_TFUNCTION: {
		int tag, ref;
		lua_pushvalue(L, -1);
		lua_rawget(L, SEEN_IDX);
		if (!lua_isnil(L, -1)) {
			ref = lua_tointeger(L, -1);
			tag = MAR_TREF;
			buf_write(L, (void*)&tag, MAR_CHR, buf);
			buf_write(L, (void*)&ref, MAR_I32, buf);
			lua_pop(L, 1);
		} else {
			mar_Buffer rec_buf;
			int i;
			lua_Debug ar;
			lua_pop(L, 1);
			/* pop nil */

			lua_pushvalue(L, -1);
			lua_getinfo(L, ">nuS", &ar);
			if (ar.what[0] != 'L') {
				luaL_error(L, "attempt to persist a C function '%s'", ar.name);
			}
			tag = MAR_TVAL;
			lua_pushvalue(L, -1);
			lua_pushinteger(L, (*idx)++);
			lua_rawset(L, SEEN_IDX);

			lua_pushvalue(L, -1);
			buf_init(L, &rec_buf);
			lua_dump(L, (lua_Writer)buf_write, &rec_buf);

			buf_write(L, (void*)&tag, MAR_CHR, buf);
			buf_write(L, (void*)&rec_buf.head, MAR_I32, buf);
			buf_write(L, rec_buf.data, rec_buf.head, buf);
			buf_done(L, &rec_buf);
			lua_pop(L, 1);

			lua_newtable(L);
			for (i = 1; i <= ar.nups; i++) {
				lua_getupvalue(L, -2, i);
				lua_rawseti(L, -2, i);
			}

			buf_init(L, &rec_buf);
			mar_encode_table(L, &rec_buf, idx);

			buf_write(L, (void*)&rec_buf.head, MAR_I32, buf);
			buf_write(L, rec_buf.data, rec_buf.head, buf);
			buf_done(L, &rec_buf);
			lua_pop(L, 1);
		}

		break;
	}
	case LUA_TUSERDATA: {
		int tag, ref;
		lua_pushvalue(L, -1);
		lua_rawget(L, SEEN_IDX);
		if (!lua_isnil(L, -1)) {
			ref = lua_tointeger(L, -1);
			tag = MAR_TREF;
			buf_write(L, (void*)&tag, MAR_CHR, buf);
			buf_write(L, (void*)&ref, MAR_I32, buf);
			lua_pop(L, 1);
		} else {
			mar_Buffer rec_buf;
			lua_pop(L, 1);
			/* pop nil */
			if (luaL_getmetafield(L, -1, "__persist")) {
				tag = MAR_TUSR;

				lua_pushvalue(L, -2);
				lua_pushinteger(L, (*idx)++);
				lua_rawset(L, SEEN_IDX);

				lua_pushvalue(L, -2);
				lua_call(L, 1, 1);
				if (!lua_isfunction(L, -1)) {
					luaL_error(L, "__persist must return a function");
				}
				lua_newtable(L);
				lua_pushvalue(L, -2);
				lua_rawseti(L, -2, 1);
				lua_remove(L, -2);

				buf_init(L, &rec_buf);
				mar_encode_table(L, &rec_buf, idx);

				buf_write(L, (void*)&tag, MAR_CHR, buf);
				buf_write(L, (void*)&rec_buf.head, MAR_I32, buf);
				buf_write(L, rec_buf.data, rec_buf.head, buf);
				buf_done(L, &rec_buf);
			} else {
				luaL_error(L, "attempt to encode userdata (no __persist hook)");
			}
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

static int mar_encode_table(lua_State *L, mar_Buffer *buf, size_t *idx) {
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

void mar_decode_value(lua_State *L, const char *buf, size_t len, const char **p,
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
		} else if (tag == MAR_TVAL) {
			mar_next_len(l, uint32_t);
			lua_newtable(L);
			lua_pushvalue(L, -1);
			lua_rawseti(L, SEEN_IDX, (*idx)++);
			mar_decode_table(L, *p, l, idx);
			mar_incr_ptr(l);
		} else if (tag == MAR_TUSR) {
			mar_next_len(l, uint32_t);
			lua_newtable(L);
			mar_decode_table(L, *p, l, idx);
			lua_rawgeti(L, -1, 1);
			lua_call(L, 0, 1);
			lua_remove(L, -2);
			lua_pushvalue(L, -1);
			lua_rawseti(L, SEEN_IDX, (*idx)++);
			mar_incr_ptr(l);
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
			mar_next_len(l, uint32_t);
			lua_newtable(L);
			mar_decode_table(L, *p, l, idx);
			lua_rawgeti(L, -1, 1);
			lua_call(L, 0, 1);
			lua_remove(L, -2);
			lua_pushvalue(L, -1);
			lua_rawseti(L, SEEN_IDX, (*idx)++);
			mar_incr_ptr(l);
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

static mar_Buffer global_mar_buffer;
static bool global_buffer_initialized = false;

int mar_encode(lua_State* L) {
	size_t idx, len;

	if (!global_buffer_initialized) {
		buf_init(L, &global_mar_buffer);
		global_buffer_initialized = true;
	} else {
		global_mar_buffer.seek = 0;
		global_mar_buffer.head = 0;
	}

	if (lua_isnone(L, 1)) {
		lua_pushnil(L);
	}
	if (lua_isnoneornil(L, 2)) {
		lua_newtable(L);
	} else if (!lua_istable(L, 2)) {
		luaL_error(L, "bad argument #2 to encode (expected table)");
	}

	len = lua_objlen(L, 2);
	lua_newtable(L);
	for (idx = 1; idx <= len; idx++) {
		lua_rawgeti(L, 2, idx);
		lua_pushinteger(L, idx);
		lua_rawset(L, SEEN_IDX);
	}

	lua_pushvalue(L, 1);
	mar_encode_value(L, &global_mar_buffer, -1, &idx);
	//remove seen table and value copy
	lua_pop(L, 2);

	return 0;
}

int mar_decode(lua_State* L) {
	size_t l, idx, len;

	if (!global_buffer_initialized) {
		buf_init(L, &global_mar_buffer);
		global_buffer_initialized = true;
	}

	const char *p;
	const char *s = global_mar_buffer.data;

	if (lua_isnoneornil(L, 1)) {
		lua_newtable(L);
	} else if (!lua_istable(L, 1)) {
		luaL_error(L, "bad argument #1 to decode (expected table)");
	}

	len = lua_objlen(L, 1);
	lua_newtable(L);
	for (idx = 1; idx <= len; idx++) {
		lua_rawgeti(L, 1, idx);
		lua_rawseti(L, SEEN_IDX, idx);
	}

	p = s;
	mar_decode_value(L, global_mar_buffer.data, global_mar_buffer.head, &p,
			&idx);
	return 1;
}

int mar_clone(lua_State* L) {
	mar_encode(L);
	lua_replace(L, 1);
	mar_decode(L);
	return 1;
}

/* lcommon adapter code starts here */
void lua_serialize(SerializeBuffer& serialize, lua_State* L, int idx) {
	if (lua_isnil(L,idx)) {
		serialize.write_int(0);
		return;
	}

	lua_pushvalue(L, idx);
	lua_pushcfunction(L, mar_encode);
	lua_insert(L, -2);

	lua_call(L, 1, 0);
	int size = global_mar_buffer.head;
	serialize.write(size);
	serialize.write_raw(global_mar_buffer.data, size);
}

void lua_deserialize(SerializeBuffer& serialize, lua_State* L) {

	if (!global_buffer_initialized) {
		buf_init(L, &global_mar_buffer);
		global_buffer_initialized = true;
	}

	int size;
	serialize.read_int(size);

	if (size == 0) {
		lua_pushnil(L);
		return;
	}

	global_mar_buffer.seek = 0;
	global_mar_buffer.head = size;

	if (size > global_mar_buffer.size) {
		global_mar_buffer.size = size;
		global_mar_buffer.data = (char*)malloc(size);
	}

	serialize.read_raw(global_mar_buffer.data, size);

	lua_pushcfunction(L, mar_decode);

	lua_call(L, 0, 1);
}

#include <luawrap/LuaValue.h>

void lua_serialize(SerializeBuffer& serializer, lua_State* L,
		const LuaValue& value) {
	serializer.write_byte(value.empty());

	if (!value.empty()) {
		value.push(L);
		lua_serialize(serializer, L, -1);
	}
}

void lua_deserialize(SerializeBuffer& serializer, lua_State* L,
		LuaValue& value) {
	bool isnull;
	serializer.read_byte(isnull);

	if (!isnull) {
		lua_serialize(serializer, L, -1);
		value.pop(L);
	} else {
		value = LuaValue();
	}
}

