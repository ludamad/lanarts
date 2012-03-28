/*
 * UnitSet.h
 *
 *  Created on: Jun 28, 2011
 *      Author: 100397561
 */

#ifndef UNITSET_H_
#define UNITSET_H_

#include "../util/settools.h"
#include "../util/geometry.h"
#include "objects/GameInst.h"
#include <vector>

class GameState;

//Collection class that keeps track of game objects and assigns instance-id's.
//Deallocates destroyed objects at end of step.
//Use instance IDs to refer to objects that may or may not still be alive.

class GameInstSet {
public:
	enum {
		DEFAULT_CAPACITY = 256,
		REGION_SIZE = 96//cell is REGION_SIZE by REGION_SIZE
	};
	GameInstSet(int w, int h, int capacity = DEFAULT_CAPACITY);
	~GameInstSet();

	obj_id add(GameInst* inst);
	void remove(GameInst* inst);
	void step(GameState* state);

	//Returns NULL if no unit found
	GameInst* get_by_id(int id);
	GameInst* get_by_Pos(const Pos& c);

	GameInst* object_nearest_test(GameInst* obj, int max_radius, col_filterf f = NULL);
	int object_radius_test(GameInst* obj, GameInst** objs = NULL, int obj_cap = 0, col_filterf f = NULL, int x=-1, int y=-1, int radius=-1);

	size_t size(){ return unit_amnt; }
	std::vector<GameInst*> to_vector();
	void clear();
private:
	void reallocate_hashset_();

	struct InstState{
		GameInst* inst;
		//These pointers are invalidated upon hashmap reallocation
		InstState* next_in_grid, *prev_in_grid;
		//Used in settools.h
		void operator=(GameInst* inst){
			this->inst = inst;
			next_in_grid = NULL;
			prev_in_grid = NULL;
		}
	};
	void update_instance(InstState* state, GameInst* inst);

	friend class GameInstSetFunctions;
	//Destroyed objects marked for deletion
	std::vector<GameInst*> deallocation_list;
	//Hashset portion
	int next_id, unit_amnt, unit_capacity;
	InstState* unit_set;

	//Grid portion
	int grid_w, grid_h;
	//Holds units IDs, 0 if empty
	int* unit_grid;
};

#endif /* UNITSET_H_ */
