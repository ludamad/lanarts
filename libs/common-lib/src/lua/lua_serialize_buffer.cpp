/*
 * lua_serialize_buffer.cpp:
 *  Implements wrapping for 'SerializeBuffer' in lua.
 */

#include <luawrap/luawrap.h>
#include <luawrap/macros.h>

#include "luaserialize.h"
#include "SerializeBuffer.h"

LuaValue lua_serializebuffermetatable(lua_State* L) {
	LUAWRAP_SET_TYPE(SerializeBuffer&);

	LuaValue meta = luameta_new(L, "SerializeBuffer");
	LuaValue methods = luameta_constants(meta);
	LuaValue getters = luameta_getters(meta);

//	LUAWRAP_METHOD(methods, write, lua_serialize(OBJ, L, lua_gettop(L) - 1));
//	LUAWRAP_METHOD(methods, read, lua_deserialize(OBJ, L, lua_gettop(L) - 1));
	LUAWRAP_METHOD(methods, write_int, OBJ.write_int(lua_tointeger(L, 2)));
	LUAWRAP_METHOD(methods, write_double, OBJ.write(lua_tonumber(L, 2)));
	LUAWRAP_METHOD(methods, write_raw,
			size_t size;
			const char* str = lua_tolstring(L, 2, &size);
			OBJ.write_raw(str, size)
	);
	LUAWRAP_METHOD(methods, move_read_position, OBJ.move_read_position(lua_tointeger(L, 2)));

	LUAWRAP_GETTER(methods, read_int, OBJ.read_int());
	LUAWRAP_GETTER(methods, read_double, OBJ.read_int());
	LUAWRAP_METHOD(methods, read_raw,
			int size = lua_tointeger(L, 2);
			char* buf = (char*)malloc(size);
			OBJ.read_raw(buf, size);
			lua_pushlstring(L, buf, size);
			free(buf)
	);

	LUAWRAP_GETTER(methods, read_position, OBJ.read_position());
	LUAWRAP_METHOD(getters, raw_buffer, lua_pushlstring(L, OBJ.data(), OBJ.size()));

	luameta_gc<SerializeBuffer>(meta);

	return meta;
}

static int serializebuffer_create(lua_State* L) {
	bool noarg = lua_isnoneornil(L, 1);
	void* data = lua_newuserdata(L, sizeof(SerializeBuffer));
	luameta_push(L, &lua_serializebuffermetatable);
	lua_setmetatable(L,-2);

	if (noarg) {
		new ((SerializeBuffer*)data) SerializeBuffer();
	} else {
		LuaStackValue args(L, 1);
		if (!args["output_file"].isnil()) {
			const char* filename = args["output_file"].to_str();
			new ((SerializeBuffer*)data) SerializeBuffer(fopen(filename, "wb"), SerializeBuffer::OUTPUT, true);
		} else if (!args["input_file"].isnil()) {
			const char* filename = args["input_file"].to_str();
			new ((SerializeBuffer*)data) SerializeBuffer(fopen(filename, "rb"), SerializeBuffer::INPUT, true);
		}
	}
	return 1;
}

LuaValue lua_serializebuffer_type(lua_State *L) {
	luawrap::install_userdata_type<SerializeBuffer, &lua_serializebuffermetatable>();

	LuaValue type(L);
	type.newtable();
	type["create"].bind_function(serializebuffer_create);

	return type;
}
