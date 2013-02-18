/*
 * ResourceEntryBase.cpp:
 *  The base of various data entries, allows for operations common to all describable entries
 */

#include "ResourceEntryBase.h"

ResourceEntryBase::ResourceEntryBase() {
	this->id = -1;
}

ResourceEntryBase::~ResourceEntryBase() {
}

void ResourceEntryBase::convert_lua() {
}

void ResourceEntryBase::init( lua_State* L, int id, const std::string& name ) {
	this->id = id;
	this->name = name;

	_luatable.init(L);
}


LuaValue ResourceEntryBase::lua_table() const {
	return _luatable;
}
