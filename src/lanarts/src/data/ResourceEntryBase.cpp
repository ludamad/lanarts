/*
 * ResourceEntryBase.cpp:
 *  The base of various data entries, allows for operations common to all describable entries
 */

#include "ResourceEntryBase.h"

ResourceEntryBase::~ResourceEntryBase() {
}

void ResourceEntryBase::parse_lua() {
}

LuaValue ResourceEntryBase::lua_table() const {
	return _luatable;
}
