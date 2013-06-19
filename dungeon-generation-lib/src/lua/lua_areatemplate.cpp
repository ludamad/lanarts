/*
 * lua_areatemplate.cpp:
 *  Area template lua bindings
 */

#include <luawrap/luawrap.h>
#include <luawrap/members.h>
#include <luawrap/macros.h>

#include "AreaTemplate.h"

#include "lua_ldungeon_impl.h"

#include "Map.h"
#include "map_fill.h"
#include "map_check.h"

namespace ldungeon_gen {

	static int strlinewidth(const std::string& data) {
		for (int i = 0; i < data.size(); i++) {
			if (isspace(data[i])) {
				return i;
			}
		}
		return data.size();
	}

	static int strlineheight(const std::string& data, int width) {
		int chars_since_space = 0;
		int height = 0;
		int null_term_len = data.size() + 1; // Size including null-terminator
		for (int i = 0; i < null_term_len; i++) {
			if (!data[i] || isspace(data[i])) {
				if (chars_since_space != width) {
					printf(" Got %d vs %d\n", chars_since_space, width);
					luawrap::error("Non-uniform grid passed to area template");
				}
				chars_since_space = 0;
				do {
					i++;
				} while (i < null_term_len && isspace(data[i]));
				height++;
			}
			chars_since_space++;
		}
		return height;
	}

	static Glyph glyph_get(LuaStackValue glyph) {
		LuaValue on_placement = luawrap::defaulted(glyph["on_placement"], LuaValue());
		return Glyph(lua_square_get(glyph), on_placement);
	}

	static LuaStackValue area_template_create(LuaStackValue args) {
		const char* data = args["data"].to_str();
		int width = strlinewidth(data);
		int height = strlineheight(data, width);
		int data_width = width + 1;
		AreaTemplatePtr temp(
				new AreaTemplate(data, data_width, Size(width, height)));
		lua_State* L = args.luastate();

		args["legend"].push();
		lua_pushnil(L); /* first key */
		while (lua_next(L, -2) != 0) {
			const char* key = lua_tostring(L, -2);
			if (strlen(key) != 1) {
				luawrap::error(
						"Expecting characters only in area template's legend");
			}
			temp->define_glyph(*key, glyph_get(LuaStackValue(L, -1)));
			lua_pop(L, 1);
		}
		lua_pop(L, 1); /* pop test table */

		luawrap::push(L, temp);
		return LuaStackValue(L, -1);
	}

	static void area_template_apply(AreaTemplatePtr temp, LuaStackValue args) {
		using namespace luawrap;
		bool flipX = defaulted(args["flip_x"], false),
				flipY = defaulted(args["flip_y"], false);
		bool create_subgroup = defaulted(args["create_subgroup"], true);
		temp->apply(args["map"].as<MapPtr>(), args["group"].to_int(),
				args["top_left_xy"].as<Pos>(), flipX, flipY, create_subgroup);
	}

	LuaValue lua_areatemplatemetatable(lua_State* L) {
		LUAWRAP_SET_TYPE(AreaTemplatePtr);

		LuaValue meta = luameta_new(L, "MapGen.AreaTemplate");
		LuaValue methods = luameta_constants(meta);
		methods["apply"].bind_function(area_template_apply);

		return meta;
	}

	void lua_register_areatemplate(const LuaValue& module) {
		luawrap::install_userdata_type<AreaTemplatePtr, lua_areatemplatemetatable>();
		module["area_template_create"].bind_function(area_template_create);
	}
}
