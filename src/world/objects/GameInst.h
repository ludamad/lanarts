/*
 * GameInst.h
 *
 *  Created on: 2011-09-27
 *      Author: 100397561
 */

#ifndef GAMEINST_H_
#define GAMEINST_H_

//struct for game coordinates
struct Coord {
	int x, y;
	inline Coord(int x, int y) : x(x), y(y) { }
	bool operator=(const Coord& c){
		return x == c.x && y == c.y;
	}
};

struct GameState;
//Base class for game instances
class GameInst {
public:
	GameInst(int x, int y, int radius, bool solid=true) :
		id(0), x(x), y(y), radius(radius), solid(solid), destroyed(false) {}
	virtual ~GameInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
public:
	/*Should probably keep these public, many functions operate on these*/
	int id;
	int x, y, radius;
	bool solid, destroyed;
};


#endif /* GAMEINST_H_ */
