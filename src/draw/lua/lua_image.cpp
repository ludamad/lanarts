/*
 * lua_image.cpp:
 * 	Bindings for ldraw::Image
 */

#include <SLB/Class.hpp>
#include <SLB/Manager.hpp>

#include <common/lua/slb_funcproperty.h>

#include "../Image.h"

#include "lua_ldraw.h"

static int luacfunc_draw(lua_State* L) {
	using namespace ldraw;
	using namespace SLB;

	const char* LERR_MSG =
			"Incorrect Image::draw usage, use img:draw(position) or img:draw(options, position)";

	int nargs = lua_gettop(L);
	const Image* img = get<const Image*>(L, 1);
	if (!img) {
		luaL_error(L, LERR_MSG);
		return 0;
	}

	if (nargs == 2) {
		Pos p = get<Pos>(L, 2);
		img->draw(p);
	} else {
		luaL_error(L, LERR_MSG);
	}

	return 0;
}

void lua_register_image(lua_State* L) {
	using namespace SLB;
	using namespace ldraw;

	Manager* m = Manager::getInstance(L);
	typedef void (Image::*PosFunc)(const Posf& pos) const;
	Class<Image>("Image", m);
//	.set("draw", static_cast<PosFunc>(&Image::draw));

	ClassInfo* class_info = m->getClass("Image");
	addFuncProperty<Image, float, &Image::width>(class_info, "width");
	addFuncProperty<Image, float, &Image::height>(class_info, "height");

	class_info->set("draw", FuncCall::create(luacfunc_draw));
}

