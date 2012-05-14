/*
 * GameInstSet.cpp
 *
 *  Created on: Jun 28, 2011
 *      Author: 100397561
 */
#include <cstring>
#include <cstdio>

#include "GameInstSet.h"
#include "../util/hashset_util.h"
#include "../util/math_util.h"

#include "objects/AnimatedInst.h"

GameInst* const GAMEINST_TOMBSTONE = (GameInst*) 1;

static bool valid_inst(GameInst* inst) {
	return inst > GAMEINST_TOMBSTONE;
}

GameInstSet::GameInstSet(int w, int h, int capacity) {
	grid_w = w / REGION_SIZE + 1, grid_h = h / REGION_SIZE + 1;
	next_id = 1;
	unit_amnt = 0;
	unit_capacity = capacity;
	unit_set = new InstanceState[unit_capacity];
	memset(unit_set, 0, unit_capacity * sizeof(InstanceState));
	unit_grid = new InstanceLinkedList[grid_w * grid_h];
}
GameInstSet::~GameInstSet() {
	delete[] unit_grid;
	delete[] unit_set;
	for (int i = 0; i < deallocation_list.size(); i++) {
		delete deallocation_list[i];
	}
}
struct GameInstSetFunctions { //Helper class
	typedef GameInstSet::InstanceState V;
	static bool isNull(const V& v) {
		return v.inst == NULL;
	}
	static bool isRemoved(const V& v) {
		return v.inst == GAMEINST_TOMBSTONE;
	}
	static void remove(V& v) {
		v.inst = GAMEINST_TOMBSTONE;
	}
	static bool equal(const V& v1, int obj_id) {
		return valid_inst(v1.inst) && v1.inst->id == obj_id;
	}
	static bool equal(const V& v1, GameInst* inst) {
		return v1.inst == inst;
	}
	static bool equal(const V& v1, const V& v2) {
		return v1.inst == v2.inst;
	}
	static size_t hash(const V& v) {
		return (size_t) v.inst->id;
	}
	static size_t hash(GameInst* inst) {
		return (size_t) inst->id;
	}
	static size_t hash(obj_id id) {
		return (size_t) id;
	}
};

void GameInstSet::update_statepointer_for_reallocate_(InstanceState** stateptr){
	if (*stateptr)
		*stateptr = tset_find<GameInstSetFunctions>((*stateptr)->inst->id, unit_set, unit_capacity);
}
void GameInstSet::update_depthlist_for_reallocate_(InstanceLinkedList& list){
	update_statepointer_for_reallocate_(&list.start_of_list);
	update_statepointer_for_reallocate_(&list.end_of_list);
}
void GameInstSet::reallocate_hashset_() {
	unit_capacity *= 2;
	InstanceState* old_set = unit_set, *new_set = new InstanceState[unit_capacity];
	for (int i = 0; i < unit_capacity; i++) {
		new_set[i].inst = NULL;
	}

	tset_add_all<GameInstSetFunctions>(old_set, unit_capacity / 2, new_set,
			unit_capacity);
	unit_set = new_set;

	//Fix pointers for grid
	for (int i = 0; i < unit_capacity; i++) {
		if (new_set[i].inst) {
			update_statepointer_for_reallocate_(&new_set[i].prev_in_grid);
			update_statepointer_for_reallocate_(&new_set[i].next_in_grid);
			update_statepointer_for_reallocate_(&new_set[i].prev_same_depth);
			update_statepointer_for_reallocate_(&new_set[i].next_same_depth);
		}
	}
	DepthMap::iterator it = depthlist_map.begin();
	for (; it != depthlist_map.end(); it++){
		update_depthlist_for_reallocate_(it->second);
	}

	for (int i = 0; i < grid_w*grid_h; i++){
		update_depthlist_for_reallocate_(unit_grid[i]);
	}

	delete[] old_set;

}
static int get_xyind(const Pos& c, int grid_w) {
	return (c.y / GameInstSet::REGION_SIZE) * grid_w
			+ c.x / GameInstSet::REGION_SIZE;
}
void GameInstSet::remove(GameInst* inst, bool deallocate) {
	if (inst->destroyed)
		return;
	inst->destroyed = true;
	InstanceState* state = tset_find<GameInstSetFunctions>(inst->id, unit_set,
			unit_capacity);

	InstanceLinkedList& list = unit_grid[get_xyind(Pos(inst->last_x, inst->last_y),
			grid_w)];

	remove_from_collisionlist(state, list);
	remove_from_depthlist(state, depthlist_map[inst->depth]);

	this->unit_amnt--;
	state->inst = GAMEINST_TOMBSTONE;
	if (deallocate){
		deallocation_list.push_back(inst);
	}
}

obj_id GameInstSet::add(GameInst* inst) {
	if (tset_should_resize(unit_amnt, unit_capacity))
		this->reallocate_hashset_();

	Pos c(inst->last_x, inst->last_y);
	//Will be set to the current state object in 'add'
	InstanceState* state = unit_set;

	//TODO: cause a more descriptive error
	if (inst->id != 0)
		printf("Adding instance with id not 0!\n");

	inst->id = next_id++;
	//Add an object with the assumption that this object does not currently exist (_noequal)
	if (tset_add_noequal<GameInstSetFunctions>(inst, state, unit_capacity))
		unit_amnt++;

	InstanceLinkedList& unit_list = unit_grid[get_xyind(c, grid_w)];
	add_to_collisionlist(state, unit_list);
	add_to_depthlist(state, depthlist_map[inst->depth]);

	return inst->id;
}


void GameInstSet::step(GameState* gs) {
	for (int i = 0; i < deallocation_list.size(); i++) {
		delete deallocation_list[i];
	}
	deallocation_list.clear();
	for (int i = 0; i < unit_capacity; i++) {
		GameInst* inst = unit_set[i].inst;
		if (valid_inst(inst)) {
			inst->destroyed = false;
			inst->step(gs);
			update_instance(&unit_set[i], inst);
		}
	}
}
GameInst* GameInstSet::get_by_id(int id) {
	InstanceState* is = tset_find<GameInstSetFunctions>(id, unit_set,
			unit_capacity);
	if (is)
		return is->inst;
	return NULL;
}

std::vector<GameInst*> GameInstSet::to_vector() {
	std::vector<GameInst*> ret(size(), NULL);
	for (int i = 0, j = 0; i < unit_capacity; i++) {
		GameInst* inst = unit_set[i].inst;
		if (valid_inst(inst))
			ret[j++] = inst;
	}
	DepthMap::iterator it = depthlist_map.end();
	for (; it-- != depthlist_map.begin();){
		InstanceState* state = it->second.start_of_list;
		while (state){
			ret.push_back(state->inst);
			state = state->next_same_depth;
		}

	}
	return ret;
}

unsigned int GameInstSet::hash(){
	unsigned int hash = 0xbabdabe;
	for (int i = 0, j = 0; i < unit_capacity; i++) {
		GameInst* inst = unit_set[i].inst;
		if (valid_inst(inst) && !dynamic_cast<AnimatedInst*>(inst)){
			hash ^= ((inst->x) << 16) + inst->y;
			hash *= 31337;
		}
	}
	return hash;
}
int GameInstSet::object_radius_test(GameInst* obj, GameInst** objs, int obj_cap,
		col_filterf f, int x, int y, int radius) {
	int rad = radius == -1 ? obj->radius : radius;
	x = x == -1 ? obj->x : x;
	y = y == -1 ? obj->y : y;

	int mingrid_x = (x - rad) / REGION_SIZE, mingrid_y = (y - rad)
			/ REGION_SIZE;
	int maxgrid_x = (x + rad) / REGION_SIZE, maxgrid_y = (y + rad)
			/ REGION_SIZE;
	int minx = squish(mingrid_x, 0, grid_w), miny = squish(mingrid_y, 0,
			grid_h);
	int maxx = squish(maxgrid_x, 0, grid_w), maxy = squish(maxgrid_y, 0,
			grid_h);

	int obj_n = 0;

	for (int yy = miny; yy <= maxy; yy++) {
		int index = yy * grid_w + minx;
		for (int xx = minx; xx <= maxx; xx++) {
			InstanceLinkedList& unit_list = unit_grid[index++];
			InstanceState* ptr = unit_list.start_of_list;
			if (!ptr) continue;

			while (ptr) {
				GameInst* inst = ptr->inst;
				if (obj != inst) {
					int radsqr = (inst->radius + rad) * (inst->radius + rad);
					int dx = inst->x - x, dy = inst->y - y;
					int dsqr = dx * dx + dy * dy;
					//want to test sqrt(dsqr) < orad+rad
					//therefore we test dsqr < (orad+rad)*(orad+rad)
					if (dsqr < radsqr && ((!f && inst->solid) || (f && f(obj, inst)))) {
						if (obj_cap == 0)
							return 1;
						objs[obj_n] = inst;
						obj_n++;
						if (obj_n >= obj_cap)
							return obj_n;
					}
				}
				ptr = ptr->next_in_grid;
			}
		}
	}
	return obj_n;
}

void GameInstSet::clear(){
	for (int i = 0; i < unit_capacity; i++) {
		GameInst* inst = unit_set[i].inst;
		if (valid_inst(inst)) {
			delete inst;
		}
	}
	next_id = 1;
	unit_amnt = 0;
	memset(unit_set, 0, unit_capacity * sizeof(InstanceState));
}


void GameInstSet::update_instance(InstanceState* state, GameInst* inst) {
	if (inst->destroyed)
		return;
	int old_bucket = get_xyind(Pos(inst->last_x, inst->last_y), grid_w);
	int new_bucket = get_xyind(Pos(inst->x, inst->y), grid_w);
	InstanceState* st = tset_find<GameInstSetFunctions>(inst->id, unit_set,
			unit_capacity);

	if (!st || inst != st->inst) {
		printf("Non-equal for id=%d!\n", inst->id);
	}
	if (old_bucket != new_bucket) {
		//remove from previous unit grid lookup
		remove_from_collisionlist(state, unit_grid[old_bucket]);
		//add to next unit lookup
		add_to_collisionlist(state, unit_grid[new_bucket]);
	}
	inst->last_x = inst->x, inst->last_y = inst->y;
}

void GameInstSet::add_to_depthlist(InstanceState* inst, InstanceLinkedList& list){
	inst->next_same_depth = NULL;
	if (list.start_of_list == NULL){
		inst->prev_same_depth = NULL;
		list.start_of_list = inst;
		list.end_of_list = inst;
	} else {
		inst->prev_same_depth = list.end_of_list;
		list.end_of_list->next_same_depth = inst;
		list.end_of_list = inst;
	}
}


void GameInstSet::remove_from_depthlist(InstanceState* inst, InstanceLinkedList& list){
	InstanceState* next = inst->next_same_depth;
	InstanceState* prev = inst->prev_same_depth;
	if (prev == NULL)
		list.start_of_list = next;
	else
		prev->next_same_depth = next;
	if (next == NULL)
		list.end_of_list = prev;
	else
		next->prev_same_depth = prev;
}



void GameInstSet::add_to_collisionlist(InstanceState* inst, InstanceLinkedList& list){
	inst->next_in_grid = NULL;
	if (list.start_of_list == NULL){
		inst->prev_in_grid = NULL;
		list.start_of_list = inst;
		list.end_of_list = inst;

	} else {
		inst->prev_in_grid = list.end_of_list;
		list.end_of_list->next_in_grid = inst;
		list.end_of_list = inst;
	}
}


void GameInstSet::remove_from_collisionlist(InstanceState* inst, InstanceLinkedList& list){
	InstanceState* next = inst->next_in_grid;
	InstanceState* prev = inst->prev_in_grid;
	if (prev == NULL)
		list.start_of_list = next;
	else
		prev->next_in_grid = next;
	if (next == NULL)
		list.end_of_list = prev;
	else
		next->prev_in_grid = prev;
}

