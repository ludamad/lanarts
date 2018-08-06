/*
 * ResourceEntryBase.h:
 *  The base of various data entries, allows for operations common to all describable entries
 */

#ifndef RESOURCEENTRYBASE_H_
#define RESOURCEENTRYBASE_H_

#include <string>
#include <stdexcept>

#include <luawrap/LuaValue.h>
#include <lcommon/geometry.h>

#include "lanarts_defines.h"

class ResourceError: public std::runtime_error {
public:
	ResourceError(const std::string& msg) :
					std::runtime_error(msg) {

	}
};

namespace ldraw {
	class DrawOptions;
}

class ResourceEntryBase {
public:
	// Both id & name can identifiy the resource uniquely
	int id;
	std::string name;
	std::string description;

	LuaValue raw;

	ResourceEntryBase();
	virtual ~ResourceEntryBase();
	virtual void draw(const ldraw::DrawOptions& options, Pos xy);

	virtual sprite_id get_sprite() = 0;
	virtual const char* entry_type() = 0;

	/* initialize a resource, with a table representation of the resource */
	void init(int id, const LuaValue& table);

	virtual void parse_lua_table(const LuaValue& table);
};

#endif /* RESOURCEENTRYBASE_H_ */
