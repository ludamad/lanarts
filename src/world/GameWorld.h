/* GameWorld:
 * 	Contains all the information and functionality of previously generated levels,
 * 	and which levels are currently being generated
 */

#ifndef GAMEWORLD_H_
#define GAMEWORLD_H_

struct GameLevelState;

class GameWorld {
public:
	GameWorld();
	~GameWorld();
private:
	std::vector<GameLevelState*> level_states;
};

#endif /* GAMEWORLD_H_ */
