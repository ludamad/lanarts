/*
 * MapDefEntry.h:
 *  Describes a game map definition/descriptor.
 *  This class defines an interface for describing the maps that make up the Lanarts world.
 *
    - Goals:
        - Stable interface for high level map operations in C++
        - Overall map path finding - path finding between two maps
        - Forced generation of levels
        - Forced clearing of levels
        - Generating levels in isolation of one another, e.g. as a starting point, having different connections, etc.
        - Teleport to level
        - Regenerate level
            - Make levels more like the item table
            - Stable names
 */

#ifndef GAMESTATE__MAPDEFENTRY_H
#define GAMESTATE__MAPDEFENTRY_H

#include <vector>
#include <algorithm>
#include <string>

#include <lcommon/geometry.h>

#include "lanarts_defines.h"
#include "data/ResourceEntryBase.h"
#include "data/ResourceDataSet.h"

struct GLimage;

class MapDefEntry: public ResourceEntryBase {
public:
    virtual ~MapDefEntry() {
    }
    virtual void parse_lua_table(const LuaValue& table);

    LuaValue generate_func;
    // Contentious: We want to be able to link to this map in two ways, in and out.
    // Should every map define its in and out? Implies directionality.
    // Or should we have a separate class, called MapLinkEntry?
    // This class would 
    LuaValue place_entrance_func;

};

extern ResourceDataSet<MapDefEntry*> game_item_data;

#endif