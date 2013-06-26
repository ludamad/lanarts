/*
 * lua_areatemplate.cpp:
 *  Area template lua bindings
 */

#include <lcommon/strformat.h>

#include <luawrap/macros.h>
#include <luawrap/luawrap.h>
#include <luawrap/members.h>

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

	static Size strblocksize(const char* data) {
		int width = strlinewidth(data);
		int height = strlineheight(data, width);
		return Size(width, height);
	}

	static Glyph glyph_get(LuaStackValue glyph) {
		LuaValue on_placement = luawrap::defaulted(glyph["on_placement"],
				LuaValue());
		ConditionalOperator oper = lua_conditional_operator_get(glyph);
		if (!glyph.has("remove")) {
			/* Default to removing all previous flags, least surprising!
			 * This makes defining area templates that overwrite content easy. */
			oper.oper.turn_off_bits = -1;
		}
		return Glyph(oper, on_placement);
	}

	static bool parse_legend(AreaTemplatePtr temp, LuaField legend) {
		lua_State* L = legend.luastate();

		legend.push();
		lua_pushnil(L); /* first key */
		while (lua_next(L, -2) != 0) {
			const char* key = lua_tostring(L, -2);
			if (strlen(key) != 1) {
				lua_pushstring(L,
						"Expecting characters only in area template's legend");
				lua_error(L);
				return false;
			}
			temp->define_glyph(*key, glyph_get(LuaStackValue(L, -1)));
			lua_pop(L, 1);
		}
		lua_pop(L, 1); /* pop test table */
		return true;
	}

	static void read_file_as_string(const char* filename, std::string& result) {
		result.clear();

		char buffer[2048];
		FILE* file = fopen(filename, "rb");
		while (!feof(file)) {
			int bytes_read = fread(buffer, 1, sizeof(buffer), file);
			if (bytes_read < 0) {
				luawrap::error(format("Read error of '%s'!", filename));
			}
			result.append(buffer, buffer + bytes_read);
		}
	}

	static LuaStackValue area_template_create(LuaStackValue args) {
		lua_State* L = args.luastate();

		std::string filecontents;

		const char* data;
		if (args.has("data_file")) {
			if (args.has("data")) {
				lua_pushstring(L,
						"area_template_create: Expecting either a 'data' string or a 'data_file' file, not both.");
				lua_error(L);
				return args; /* Dummy return */
			}
			read_file_as_string(args["data_file"].to_str(), filecontents);
			data = filecontents.c_str();
		} else {
			data = args["data"].to_str();
		}

		AreaTemplatePtr temp(new AreaTemplate(data, strblocksize(data)));

		if (!parse_legend(temp, args["legend"])) {
			return args; /* Dummy return */
		}

		luawrap::push(L, temp);
		return LuaStackValue(L, -1);
	}

	/* Map is only manditory component */
	static void area_template_apply(AreaTemplatePtr temp, LuaStackValue args) {
		using namespace luawrap;
		bool flipX = defaulted(args["flip_x"], false), flipY = defaulted(
				args["flip_y"], false);
		bool create_subgroup = defaulted(args["create_subgroup"], true);
		int group = defaulted(args["group"], ROOT_GROUP_ID);
		Pos top_left_xy = defaulted(args["top_left_xy"], Pos());

		temp->apply(args["map"].as<MapPtr>(), group, top_left_xy, flipX, flipY,
				create_subgroup);
	}

	LuaValue lua_areatemplatemetatable(lua_State* L) {
		LUAWRAP_SET_TYPE(AreaTemplatePtr);

		LuaValue meta = luameta_new(L, "MapGen.AreaTemplate");
		LuaValue methods = luameta_constants(meta);
		LuaValue getters = luameta_getters(meta);
		methods["apply"].bind_function(area_template_apply);

		LUAWRAP_GETTER(getters, size, OBJ->size());

		return meta;
	}

	void lua_register_areatemplate(const LuaValue& module) {
		luawrap::install_userdata_type<AreaTemplatePtr,
				lua_areatemplatemetatable>();
		module["area_template_create"].bind_function(area_template_create);
	}
}
