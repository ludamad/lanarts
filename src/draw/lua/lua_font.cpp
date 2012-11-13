/*
 * lua_font.cpp:
 *  Lua bindings for ldraw::Font
 */

#include <SLB/Manager.hpp>
#include <SLB/Class.hpp>

#include <common/lua/lua_geometry.h>

#include <lua/lua.h>

#include "lua_drawoptions.h"
#include "lua_font.h"

void lua_register_font(lua_State* L) {
	using namespace SLB;
	using namespace ldraw;

	Manager* m = Manager::getInstance(L);
	typedef void (Font::*DrawFunc)(const DrawOptions& options,
			const Posf& position, const char* str) const;
	typedef void (Font::*DrawWrappedFunc)(const DrawOptions& options,
			const Posf& position, int maxwidth, const char* str) const;
	typedef DimF (Font::*GetDrawSize)(const char* str, int maxwidth) const;

	Class<Font>("Font", m)
			.set("draw", static_cast<DrawFunc>(&Font::draw))
			.set("draw_wrapped", static_cast<DrawWrappedFunc>(&Font::draw_wrapped))
			.set("get_draw_size", static_cast<GetDrawSize>(&Font::get_draw_size));

}

