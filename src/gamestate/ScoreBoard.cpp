#include "ScoreBoard.h"

#include <luawrap/luawrap.h>
#include "stats/ClassEntry.h"

#include "GameState.h"

#include <lcommon/SerializeBuffer.h>

ScoreBoardEntry::ScoreBoardEntry() :
		character_level(0), timestamp(0), won_the_game(0), hardcore(0) {
}

void ScoreBoardEntry::lua_push(lua_State* L, const ScoreBoardEntry& entry) {
	// Push table
	lua_newtable(L);

	LuaStackValue score_table(L, -1); // Reference the new table
	score_table["name"] = entry.name;
	score_table["class_name"] = entry.class_name;
	// sprite_name not necessarily useful ATM
	score_table["sprite_name"] = entry.sprite_name;
	score_table["deaths"] = entry.score_stats.deaths;
	score_table["deepest_floor"] = entry.score_stats.deepest_floor;
	score_table["kills"] = entry.score_stats.kills;
	score_table["character_level"] = entry.character_level;
	score_table["timestamp"] = entry.timestamp;
	score_table["hardcore"] = entry.hardcore;
	score_table["won_the_game"] = entry.won_the_game;
}

ScoreBoardEntry::ScoreBoardEntry(const std::string& name,
		const std::string& sprite_name, const std::string& class_name,
		const PlayerScoreStats& score_stats, int character_level, int timestamp,
		bool won_the_game, bool hardcore) :
				name(name),
				sprite_name(sprite_name),
				class_name(class_name),
				score_stats(score_stats),
				character_level(character_level),
				timestamp(timestamp),
				won_the_game(won_the_game),
				hardcore(hardcore) {
}

ScoreBoard::ScoreBoard(const std::string& filename) :
		_filename(filename) {
}

ScoreBoard::~ScoreBoard() {
}

void ScoreBoard::store_entry(const ScoreBoardEntry& entry) {
	std::vector<ScoreBoardEntry> entries;
	read_entries(entries);
	store_entry(entries, entry);
	write_entries(entries);
}

std::vector<ScoreBoardEntry> ScoreBoard::fetch_entries() const {
	std::vector<ScoreBoardEntry> entries;
	read_entries(entries);
	return entries;
}

void ScoreBoard::store_entry(std::vector<ScoreBoardEntry>& entries,
		const ScoreBoardEntry& entry) {
	for (int i = 0; i < entries.size(); i++) {
		// Test if timestamp & name match an existing entry
		if (entries[i].name == entry.name
				&& entries[i].timestamp == entry.timestamp) {
			// Do we trump this entry ?
			if (entries[i].score_stats.deepest_floor
					<= entry.score_stats.deepest_floor
					&& entries[i].score_stats.kills
							<= entry.score_stats.kills) {
				entries[i] = entry;
			}
			return;
		}
	}

	// Timestamp & name did not match an entry, add to end
	entries.push_back(entry);
}
void ScoreBoard::read_entries(std::vector<ScoreBoardEntry>& entries) const {
	FILE* entry_file = fopen(_filename.c_str(), "rb");
	if (entry_file == NULL) {
		return; // Interpret non-existent as empty
	}

	SerializeBuffer reader(entry_file, SerializeBuffer::INPUT);

	try {
		int entry_amount;
		reader.read_int(entry_amount);

		for (int i = 0; i < entry_amount; i++) {
			ScoreBoardEntry entry;
			reader.read(entry.name);
			reader.read(entry.sprite_name);
			reader.read(entry.class_name);
			reader.read(entry.score_stats);
			reader.read(entry.character_level);
			reader.read(entry.timestamp);
			reader.read(entry.won_the_game);
			reader.read(entry.hardcore);
			entries.push_back(entry);
		}
	} catch (const SerializeBufferError& sbe) {
		// TODO: Be more graceful
		printf(
				"WARNING: Incompatible high score list was detected. The list may be overridden.\n");
		printf("Internal error: '%s'\n", sbe.what());
		entries.clear(); // Don't show corrupted entries
	}

	reader.flush();

	fclose(entry_file);
}

ScoreBoard ScoreBoard::get_instance() {
	return ScoreBoard(SCOREBOARD_FILE);
}

void ScoreBoard::write_entries(const std::vector<ScoreBoardEntry>& entries) {
	FILE* entry_file = fopen(_filename.c_str(), "wb");

	if (entry_file == NULL) {
		// Failure, just log and move on
		printf("ScoreBoard::write_entries failed to open file '%s'\n",
				_filename.c_str());
		return;
	}

	SerializeBuffer writer(entry_file, SerializeBuffer::OUTPUT);

	writer.write_int(entries.size());

	for (int i = 0; i < entries.size(); i++) {
		const ScoreBoardEntry& entry = entries[i];
		writer.write(entry.name);
		writer.write(entry.sprite_name);
		writer.write(entry.class_name);
		writer.write(entry.score_stats);
		writer.write(entry.character_level);
		writer.write(entry.timestamp);
		writer.write(entry.won_the_game);
		writer.write(entry.hardcore);
	}

	writer.flush();

	fclose(entry_file);
}

void score_board_store(GameState* gs, bool won_the_game) {
	gs->for_screens([&]() {
		PlayerInst *player = gs->local_player();

		std::string name = player->player_entry(gs).player_name;
		std::string sprite_name = res::sprite_name(player->get_sprite());
		std::string class_name = player->class_stats().class_entry().name;

		bool hardcore = !gs->game_settings().regen_on_death;
		ScoreBoardEntry entry(name, sprite_name, class_name, player->score_stats(),
							  player->class_stats().xplevel, gs->game_timestamp(), won_the_game,
							  hardcore);

		ScoreBoard::get_instance().store_entry(entry);
	});
}

std::vector<ScoreBoardEntry> score_board_fetch() {
	return ScoreBoard::get_instance().fetch_entries();
}
