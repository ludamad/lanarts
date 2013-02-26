/*
 * ResourceEntryBase.cpp:
 *  The base of various data entries, allows for operations common to all describable entries
 */

#include <luawrap/luawrap.h>

#include "ResourceEntryBase.h"

ResourceEntryBase::ResourceEntryBase() {
	this->id = -1;
}

ResourceEntryBase::~ResourceEntryBase() {
}

void ResourceEntryBase::init(int id, const LuaValue& table) {
	this->id = id;
	parse_lua_table(table);
}

void ResourceEntryBase::parse_lua_table( const LuaValue& table ) {
	this->name = table["name"].as<std::string>();
}
