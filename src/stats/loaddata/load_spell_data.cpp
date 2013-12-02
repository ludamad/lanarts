/*
 * load_spell_data.cpp:
 *  Implements spell loading routines from YAML datafiles
 */
#include <fstream>
#include <yaml-cpp/yaml.h>

#include "data/game_data.h"

#include "data/parse.h"
#include "data/yaml_util.h"

#include "lua_api/lua_yaml.h"

#include "load_stats.h"

using namespace std;

extern std::vector<SpellEntry> game_spell_data;

static LuaValue spell_table;
static void lapi_data_create_spell(const LuaStackValue& table) {
	SpellEntry entry;
	int idx = game_spell_data.size();
	entry.init(idx, table);
	spell_table[idx+1] = table;
	game_spell_data.push_back(entry);
}

LuaValue load_spell_data(lua_State* L, const FilenameList& filenames) {
	game_spell_data.clear();

	spell_table = LuaValue(L);
	spell_table.newtable();

	LuaValue data = luawrap::ensure_table(luawrap::globals(L)["Data"]);
	data["spell_create"].bind_function(lapi_data_create_spell);
	luawrap::dofile(L, "game/lanarts/spells/Spells.lua");

	return spell_table;
}
