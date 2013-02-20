#ifndef SCOREBOARD_H_
#define SCOREBOARD_H_

#include <string>

#include <lcommon/SerializeBuffer.h>

#include "objects/player/PlayerInst.h"

struct ScoreBoardEntry {
	std::string name;
	PlayerScoreStats score_stats;
	int timestamp;
	bool hardcore;
	ScoreBoardEntry(const std::string& name,
			const PlayerScoreStats& score_stats,
			int timestamp,
			bool hardcore);
};

class ScoreBoard {
public:
	ScoreBoard(const char* filename);
	~ScoreBoard();

	/* May update other entries based on name + timestamp */
	void store_entry(const ScoreBoardEntry& entry);

private:
	SerializeBuffer _serializer;
};

#endif /* SCOREBOARD_H_ */
