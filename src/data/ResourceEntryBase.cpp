/*
 * ResourceEntryBase.cpp:
 *  The base of various data entries, allows for operations common to all describable entries
 */

#include <luawrap/luawrap.h>
#include <luawrap/luawraperror.h>

#include <lcommon/strformat.h>

#include "ResourceEntryBase.h"

ResourceEntryBase::ResourceEntryBase() {
	this->id = -1;
}

ResourceEntryBase::~ResourceEntryBase() {
}

void ResourceEntryBase::init(int id, const LuaValue& table) {
	this->id = id;
	lua_State* L = table.luastate();
	/* Store the lua stack size to better recover from errors*/
	int lua_stack_size = lua_gettop(L);

	try {
		table["id"] = this->id;

		this->name = table["name"].as<std::string>();
		this->description = luawrap::set_if_nil(table, "description",
				std::string());

		this->lua_representation = table;

		parse_lua_table(table);
	} catch (const luawrap::Error& error) {
		std::string resource_name = "<Unknown>";
		if (table.has("name") && table["name"].is<const char*>()) {
			resource_name = table["name"].to_str();
		}

		std::string location;
		lua_Debug debug;
		if (lua_getstack(table.luastate(), 1, &debug)) {
			lua_getinfo(table.luastate(), "nSl", &debug);
			location = format("%s line %d", debug.source, debug.currentline);
		} else {
			debug.source = "Unknown";
			debug.currentline = 0;
			location = "Called from C++ code:";
		}

		std::string err_msg = format("%s\n"
				"Resource error while loading %s '%s':\n%s", location.c_str(),
				str_tolower(entry_type()).c_str(), resource_name.c_str(),
				error.what());

		/* Restore at least the lua stack size, even if filled with wrong values */
		lua_settop(L, lua_stack_size);

		throw ResourceError(err_msg);
	}
}

void ResourceEntryBase::parse_lua_table(const LuaValue& table) {
}
