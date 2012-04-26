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
#include "../../util/game_basic_structs.h"

struct GameState;
//Base class for game instances

class GameInst {
public:
	GameInst(int x, int y, int radius, bool solid=true) :
		id(0), last_x(x), last_y(y), x(x), y(y), radius(radius), solid(solid), destroyed(false) {}
	virtual ~GameInst();
	/* Initialize the object further, 'id' will be set*/
	virtual void init(GameState* gs);
	/* Deinitialize the object, removing eg child instances*/
	virtual void deinit(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);

	BBox bbox(){
		return BBox(x-radius,y-radius,x+radius,y+radius);
	}
	Pos pos(){
		return Pos(x,y);
	}

public:
	/*Should probably keep these public, many functions operate on these*/
	obj_id id;
	int last_x, last_y;
	int x, y, radius;
	bool solid, destroyed;
};

typedef bool (*col_filterf)(GameInst* o1, GameInst* o2);


#endif /* GAMEINST_H_ */
