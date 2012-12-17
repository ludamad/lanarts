/*
 * lua_font.cpp:
 *  Lua bindings for ldraw::Font
 */

#include <lua.hpp>

#include <lcommon/lua_geometry.h>

#include <luawrap/LuaValue.h>
#include <luawrap/luameta.h>
#include <luawrap/types.h>
#include <luawrap/functions.h>

#include "lua_drawoptions.h"
#include "lua_font.h"

namespace ldraw {

static Font font_load(const char* filename, int height) {
	return Font(filename, height);
}

static int font_draw(const Font& f, const DrawOptions& options,
		const Posf& position, const char* str) {
	return f.draw(options, position, str);
}

static void font_draw_wrapped(const Font& f, const DrawOptions& options,
		const Posf& position, int maxwidth, const char* str) {
	f.draw_wrapped(options, position, maxwidth, str);
}

static DimF font_get_draw_size(const Font& f, const char* str, int maxwidth) {
	return f.get_draw_size(str, maxwidth);
}

LuaValue lua_fontmetatable(lua_State* L) {
	LuaValue meta = luameta_new(L, "Font");
	LuaValue methods = luameta_methods(meta);

	methods["draw"].bind_function(font_draw);
	methods["draw_wrapped"].bind_function(font_draw_wrapped);
	methods["get_draw_size"].bind_function(font_get_draw_size);


	return meta;
}

void lua_register_font(lua_State* L, const LuaValue& module) {
	using namespace ldraw;
	luawrap::install_userdata_type<Font, lua_fontmetatable>();

	module["font_load"] = luawrap::function(L, font_load);
}

}
