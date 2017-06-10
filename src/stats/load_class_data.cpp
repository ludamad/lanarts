/*
 * load_class_data.cpp:
 *  Load class stats and experience data
 */

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "lua_api/lua_api.h"

#include <luawrap/luawrap.h>

#include <lcommon/lua_lcommon.h>
#include <lcommon/strformat.h>

#include "data/game_data.h"
#include "data/parse.h"

#include "objects/EnemyEntry.h"

#include "lua_api/lua_yaml.h"

#include "ClassEntry.h"
#include "stats.h"

#include "load_stats.h"

using namespace std;

void lapi_data_create_class(const LuaStackValue& table) {
	ClassEntry entry;
	entry.init(entry.class_id, table);
	entry.class_id = game_class_data.size();
	auto& created_entry = game_class_data.new_entry(entry.name, entry, table);
	auto& cstats = created_entry.starting_stats;
	cstats.class_stats.classid = (game_class_data.size() - 1);
	cstats.init();
}

