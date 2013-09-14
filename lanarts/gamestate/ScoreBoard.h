#ifndef SCOREBOARD_H_
#define SCOREBOARD_H_

#include <string>

#include "objects/player/PlayerInst.h"

class GameState;

const char SCOREBOARD_FILE[] = "saves/score_board.score";

struct ScoreBoardEntry {
	std::string name;
	std::string sprite_name;
	std::string class_name;

	PlayerScoreStats score_stats;

	int character_level;
	int timestamp; // When the game started, used to uniquely identify a character
	bool won_the_game; // Well, did we win ?
	bool hardcore; // Playing on hardcore mode ?

	static void lua_push(lua_State* L, const ScoreBoardEntry& entry);

	ScoreBoardEntry();
	ScoreBoardEntry(const std::string& name, const std::string& sprite_name,
			const std::string& class_name, const PlayerScoreStats& score_stats,
			int character_level, int timestamp, bool won_the_game,
			bool hardcore);
};

// Pseudo-singleton, is backed by a single file
class ScoreBoard {
public:
	static ScoreBoard get_instance();
	~ScoreBoard();

	/* May update other entries based on name + timestamp */
	void store_entry(const ScoreBoardEntry& entry);

	std::vector<ScoreBoardEntry> fetch_entries() const;

private:
	ScoreBoard(const std::string& filename);

	static void store_entry(std::vector<ScoreBoardEntry>& entries,
			const ScoreBoardEntry& entry);
	void read_entries(std::vector<ScoreBoardEntry>& entries) const;
	void write_entries(const std::vector<ScoreBoardEntry>& entries);

	std::string _filename;
};

/* Utility methods */
void score_board_store(GameState* gs, bool won_the_game = false);
std::vector<ScoreBoardEntry> score_board_fetch();

#endif /* SCOREBOARD_H_ */
