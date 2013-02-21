#include "ScoreBoard.h"

#include "GameState.h"

#include <lcommon/SerializeBuffer.h>

ScoreBoardEntry::ScoreBoardEntry() :
		character_level(0), timestamp(0), hardcore(0) {
}

ScoreBoardEntry::ScoreBoardEntry(const std::string& name,
		const PlayerScoreStats& score_stats, int character_level, int timestamp,
		bool hardcore) :
				name(name),
				score_stats(score_stats),
				character_level(character_level),
				timestamp(timestamp),
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

	SerializeBuffer reader = SerializeBuffer::file_reader(entry_file);

	int entry_amount;
	reader.read_int(entry_amount);

	for (int i = 0; i < entry_amount; i++) {
		ScoreBoardEntry entry;
		reader.read(entry.name);
		reader.read(entry.score_stats);
		reader.read(entry.character_level);
		reader.read(entry.timestamp);
		reader.read(entry.hardcore);
		entries.push_back(entry);
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

	SerializeBuffer writer = SerializeBuffer::file_writer(entry_file);

	writer.write_int(entries.size());

	for (int i = 0; i < entries.size(); i++) {
		const ScoreBoardEntry& entry = entries[i];
		writer.write(entry.name);
		writer.write(entry.score_stats);
		writer.write(entry.character_level);
		writer.write(entry.timestamp);
		writer.write(entry.hardcore);
	}

	writer.flush();

	fclose(entry_file);
}

void score_board_store(GameState* gs) {
	PlayerInst* player = gs->local_player();
	bool hardcore = !gs->game_settings().regen_on_death;
	ScoreBoardEntry entry(player_name(gs, player), player->score_stats(),
			player->class_stats().xplevel, gs->game_timestamp(), hardcore);

	ScoreBoard::get_instance().store_entry(entry);
}

std::vector<ScoreBoardEntry> score_board_fetch() {
	return ScoreBoard::get_instance().fetch_entries();
}
