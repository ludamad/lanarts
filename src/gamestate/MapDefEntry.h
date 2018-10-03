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

 Want one primitive, or a simple hierarchy at most:
   - Filter:
     + Binary classifier on a map that allows for the following ops
     place(filter, feature) -> success
     apply(filter, operator) -> void
     matches(filter, selector) -> success
   - Region:
     + Full map, that allows all Filter ops plus:
     combine(connection_scheme, region...) -> success
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
};

// All portals belong to a specific MapLinkEntry
class MapLinkEntry {
public:
    std::string exit_map, enter_map;

    // Doesn't quite work. Each map has a different scheme for giving potential locations.
    // Schema:
    // - Instantiate place_exit_map_portals
    // - This creates a placement y/n function on a rectangular area
    // - It is the job of the placement function to shift this to an appropriate position
    LuaValue place_exit_map_portals;
    LuaValue place_enter_map_portals;
};

extern ResourceDataSet<MapDefEntry> map_def_entries;
extern std::vector<MapLinkEntry> map_link_entries;

#endif