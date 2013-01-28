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
#include <luawrap/members.h>

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

static int font_height(lua_State* L) {
	Font* f = luawrap::get<Font*>(L, 1);
	return f->height();
}

static int font_get_draw_size(lua_State* L) {
	Font f = luawrap::get<const Font&>(L, 1);
	const char* str = luaL_checkstring(L, 2);
	int maxwidth = (lua_gettop(L) >= 3) ? luaL_checkinteger(L, 3) : -1;

	DimF dim = f.get_draw_size(str, maxwidth);
	luawrap::push(L, dim);
	return 1;
}

static int font_get_draw_width(const Font& font, const char* text) {
	return font.get_draw_size(text).w;
}


LuaValue lua_fontmetatable(lua_State* L) {
	LuaValue meta = luameta_new(L, "Font");
	LuaValue methods = luameta_constants(meta);
	LuaValue getters = luameta_getters(meta);

	getters["height"].bind_function(font_height);

	methods["draw"].bind_function(font_draw);
	methods["draw_wrapped"].bind_function(font_draw_wrapped);
	methods["draw_size"].bind_function(font_get_draw_size);
	methods["draw_width"].bind_function(font_get_draw_width);

	luameta_gc<Font>(meta);

	return meta;
}

void lua_register_font(lua_State* L, const LuaValue& module) {
	using namespace ldraw;
	luawrap::install_userdata_type<Font, lua_fontmetatable>();

	module["font_load"] = luawrap::function(L, font_load);
}

}
