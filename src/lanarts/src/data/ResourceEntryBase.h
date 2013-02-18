/*
 * ResourceEntryBase.h:
 *  The base of various data entries, allows for operations common to all describable entries
 */

#ifndef RESOURCEENTRYBASE_H_
#define RESOURCEENTRYBASE_H_

#include <string>

#include <luawrap/LuaValue.h>

#include "lanarts_defines.h"

class ResourceEntryBase {
public:
	// Both id & name can identifiy the resource uniquely
	int id;
	std::string name;

	std::string description;

	ResourceEntryBase();
	virtual ~ResourceEntryBase();

	virtual sprite_id get_sprite() = 0;
	virtual const char* entry_type() = 0;
	virtual void convert_lua();

	virtual void init( lua_State* L, int id, const std::string& name );

	LuaValue lua_table() const;
private:
	LuaValue _luatable;
};

#endif /* RESOURCEENTRYBASE_H_ */
