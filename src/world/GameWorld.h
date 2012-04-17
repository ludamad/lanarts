/* GameWorld:
 * 	Contains all the information and functionality of previously generated levels,
 * 	and which levels are currently being generated
 */

#ifndef GAMEWORLD_H_
#define GAMEWORLD_H_
#include <vector>
#include "../util/geometry.h"

struct GameState;
struct GameLevelState;

class GameWorld {
public:
	GameWorld(int width, int height);
	~GameWorld();
	GameLevelState* get_level(int roomid);
	void step(GameState* gs);
	void level_move(int id, int roomid1, int roomid2);
	void set_current_level(int roomid);
	void set_current_level_lazy(int roomid);
	GameLevelState *get_current_level() {
		return lvl;
	}

	int get_current_level_id() {
		return current_room_id;
	}

	bool tile_radius_test(int x, int y, int rad, bool issolid = true,
			int ttype = -1, Pos* hitloc = NULL, int roomid = -1);

private:
	void connect_entrance_to_exit(int roomid1, int roomid2);
	int w, h;
	int current_room_id, next_room_id;
	GameLevelState* lvl;
	std::vector<GameLevelState*> level_states;
};

#endif /* GAMEWORLD_H_ */
