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

#include "objects/enemy/EnemyEntry.h"

#include "lua_api/lua_yaml.h"

#include "../ClassEntry.h"
#include "../stats.h"

#include "load_stats.h"

using namespace std;

static void lapi_data_create_class(const LuaStackValue& table) {
	ClassEntry entry;
	entry.init(entry.class_id, table);
	entry.class_id = game_class_data.size();
	game_class_data.push_back(entry);
}

LuaValue load_class_data(lua_State* L, const FilenameList& filenames) {
	LuaValue data = luawrap::ensure_table(luawrap::globals(L)["Data"]);
	data["class_create"].bind_function(lapi_data_create_class);
	luawrap::dofile(L, "modules/lanarts/classes/Classes.lua");

	LuaValue ret(L);
	ret.newtable();

	for (int i = 0; i < game_class_data.size(); i++) {
		CombatStats& cstats = game_class_data[i].starting_stats;
		cstats.class_stats.classid = i;
		cstats.init();
	}

	return ret;
}
