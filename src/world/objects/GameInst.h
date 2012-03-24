/*
 * Life cycle of GameInst initialization:
 * ->Constructor
 * ->GameState::add_instance(obj) called
 * 		->GameInstSet::add(obj) called from add_instance, sets id
 * 		->GameInst::init(GameState) called from add_instance, does further initialization
 * ->
 */
#ifndef GAMEINST_H_
#define GAMEINST_H_

#include <cassert>

typedef int obj_id;

//struct for game coordinates
struct Coord {
	int x, y;
	inline Coord(){}
	inline Coord(int x, int y) : x(x), y(y) { }
	bool operator=(const Coord& c){
		return x == c.x && y == c.y;
	}
};

#define LANARTS_ASSERT(x) assert(x)

struct GameState;
//Base class for game instances


class GameInst {
public:
	GameInst(int x, int y, int radius, bool solid=true) :
		id(0), last_x(x), last_y(y), x(x), y(y), radius(radius), solid(solid), destroyed(false) {}
	virtual ~GameInst();
	/* Initialize the object further, 'id' will be set*/
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
public:
	/*Should probably keep these public, many functions operate on these*/
	obj_id id;
	int last_x, last_y;
	int x, y, radius;
	bool solid, destroyed;
};

typedef bool (*col_filterf)(GameInst* o1, GameInst* o2);


#endif /* GAMEINST_H_ */
