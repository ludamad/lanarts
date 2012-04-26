/*
 * GameInstSet.cpp
 *
 *  Created on: Jun 28, 2011
 *      Author: 100397561
 */

#include "GameInstSet.h"
#include "../util/hashset_util.h"
#include "../util/math_util.h"
#include <cstring>
#include <cstdio>

GameInst* const GAMEINST_TOMBSTONE = (GameInst*) 1;

static bool valid_inst(GameInst* inst) {
	return inst > GAMEINST_TOMBSTONE;
}

GameInstSet::GameInstSet(int w, int h, int capacity) {
	grid_w = w / REGION_SIZE + 1, grid_h = h / REGION_SIZE + 1;
	next_id = 1;
	unit_amnt = 0;
	unit_capacity = capacity;
	unit_set = new InstState[unit_capacity];
	memset(unit_set, 0, unit_capacity * sizeof(InstState));
	unit_grid = new int[grid_w * grid_h];
	memset(unit_grid, 0, grid_w * grid_h * sizeof(int));
}
GameInstSet::~GameInstSet() {
	delete[] unit_grid;
	delete[] unit_set;
	for (int i = 0; i < deallocation_list.size(); i++) {
		delete deallocation_list[i];
	}
}
struct GameInstSetFunctions { //Helper class
	typedef GameInstSet::InstState V;
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

void GameInstSet::reallocate_hashset_() {
	unit_capacity *= 2;
	InstState* old_set = unit_set, *new_set = new InstState[unit_capacity];
	for (int i = 0; i < unit_capacity; i++) {
		new_set[i].inst = NULL;
	}

	tset_add_all<GameInstSetFunctions>(old_set, unit_capacity / 2, new_set,
			unit_capacity);
	unit_set = new_set;

	//Fix pointers for grid
	for (int i = 0; i < unit_capacity; i++) {
		if (!new_set[i].inst)
			continue;
		InstState** uprev = &new_set[i].prev_in_grid;
		if (*uprev)
			*uprev = tset_find<GameInstSetFunctions>((*uprev)->inst->id,
					unit_set, unit_capacity);
		InstState** unext = &new_set[i].next_in_grid;
		if (*unext)
			*unext = tset_find<GameInstSetFunctions>((*unext)->inst->id,
					unit_set, unit_capacity);
	}

	delete[] old_set;

}
static inline int get_xyind(const Pos& c, int grid_w) {
	return (c.y / GameInstSet::REGION_SIZE) * grid_w
			+ c.x / GameInstSet::REGION_SIZE;
}
void GameInstSet::remove(GameInst* inst, bool deallocate) {
	if (inst->destroyed)
		return;
	inst->destroyed = true;
	InstState* state = tset_find<GameInstSetFunctions>(inst->id, unit_set,
			unit_capacity);

//	InstState* is = tset_find<GameInstSetFunctions> (id, unit_set, unit_capacity);
	obj_id* start_id = &unit_grid[get_xyind(Pos(inst->last_x, inst->last_y),
			grid_w)];
	InstState* next = state->next_in_grid;
	InstState* prev = state->prev_in_grid;
	if (*start_id == inst->id) {
		if (next)
			*start_id = next->inst->id;
		else
			*start_id = 0;
	}
	if (prev)
		prev->next_in_grid = next;
	if (next)
		next->prev_in_grid = prev;
	this->unit_amnt--;
	state->inst = GAMEINST_TOMBSTONE;
	if (deallocate){
		deallocation_list.push_back(inst);
	}
}

obj_id GameInstSet::add(GameInst* inst) {
	if (tset_should_resize(unit_amnt, unit_capacity))
		this->reallocate_hashset_();

	InstState* unit_state;
	Pos c(inst->last_x, inst->last_y);
	InstState* data = unit_set;

	//TODO: cause a more descriptive error
	if (inst->id != 0)
		printf("Adding instance with id not 0!\n");

	inst->id = next_id++;
	if (tset_add_noequal<GameInstSetFunctions>(inst, data, unit_capacity))
		unit_amnt++;

	obj_id* unit_id = &unit_grid[get_xyind(c, grid_w)];
	if (*unit_id) {
		unit_state = tset_find<GameInstSetFunctions>(*unit_id, unit_set,
				unit_capacity);
		while (unit_state->next_in_grid != NULL) {
			unit_state = unit_state->next_in_grid;
		}
		unit_state->next_in_grid = data;
	} else {
		*unit_id = data->inst->id;
		unit_state = NULL;
	}

	data->prev_in_grid = unit_state;
	data->next_in_grid = NULL;

	return inst->id;
}

void GameInstSet::update_instance(InstState* state, GameInst* inst) {
	if (inst->destroyed)
		return;
	int old_bucket = get_xyind(Pos(inst->last_x, inst->last_y), grid_w);
	int new_bucket = get_xyind(Pos(inst->x, inst->y), grid_w);
	InstState* st = tset_find<GameInstSetFunctions>(inst->id, unit_set,
			unit_capacity);
	bool has_id = false;
	{
		InstState* unit_state;
		obj_id* unit_id = &unit_grid[old_bucket];
		unit_state = tset_find<GameInstSetFunctions>(*unit_id, unit_set,
				unit_capacity);
		while (unit_state->next_in_grid != NULL) {
			if (unit_state->inst->id == inst->id)
				has_id = true;
			unit_state = unit_state->next_in_grid;
		}
		if (unit_state->inst->id == inst->id)
			has_id = true;
	}
	if (!st || inst != st->inst || !has_id) {
		printf("Non-equal for id=%d!\n", inst->id);
	}
	if (old_bucket != new_bucket) {
		//remove from previous unit grid lookup
		obj_id* start_id = &unit_grid[old_bucket];
		InstState* next = state->next_in_grid;
		InstState* prev = state->prev_in_grid;
		if (*start_id == inst->id) {
			if (next)
				*start_id = next->inst->id;
			else
				*start_id = 0;
		}
		if (prev)
			prev->next_in_grid = next;
		if (next)
			next->prev_in_grid = prev;
		//add to next unit lookup
		InstState* unit_state;
		obj_id* unit_id = &unit_grid[new_bucket];
		if (*unit_id) {
			unit_state = tset_find<GameInstSetFunctions>(*unit_id, unit_set,
					unit_capacity);
			while (unit_state->next_in_grid != NULL) {
				unit_state = unit_state->next_in_grid;
			}
			unit_state->next_in_grid = state;
		} else {
			*unit_id = inst->id;
			unit_state = NULL;
		}
		state->prev_in_grid = unit_state;
		state->next_in_grid = NULL;
	}
	inst->last_x = inst->x, inst->last_y = inst->y;
}
void GameInstSet::step(GameState* gs) {
	int pre_index, post_index;
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
	InstState* is = tset_find<GameInstSetFunctions>(id, unit_set,
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
	return ret;
}
GameInst* GameInstSet::get_by_Pos(const Pos& c) {
	int start_id = unit_grid[get_xyind(c, grid_w)];
	if (!start_id)
		return NULL;
	InstState* ptr = tset_find<GameInstSetFunctions>(start_id, unit_set,
			unit_capacity);
	for (;;) {
		if (ptr->inst->x == c.x && ptr->inst->y == c.y)
			return ptr->inst;
		ptr = ptr->next_in_grid;
		if (ptr == NULL)
			return NULL;
	}
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
			obj_id start_id = unit_grid[index++];
			if (!start_id)
				continue;

			InstState* ptr = tset_find<GameInstSetFunctions>(start_id, unit_set,
					unit_capacity);
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
	memset(unit_set, 0, unit_capacity * sizeof(InstState));
	memset(unit_grid, 0, grid_w * grid_h * sizeof(int));
}

GameInst* GameInstSet::object_nearest_test(GameInst* obj, int max_radius, col_filterf f){
	/*GameInst nearest_obj = NULL;
	int nearest_sqr = max_radius*max_radius;

	int max_search = max_radius/TILE_SIZE+1;

	Pos c(obj->x/REGION_SIZE, obj->y/REGION_SIZE);

	for (int rad = 1; rad < max_search; rad++){
		int start_id = unit_grid[get_xyind(c, grid_w)];
		if (!start_id)
			continue;
		InstState* ptr = tset_find<GameInstSetFunctions>(start_id, unit_set,
				unit_capacity);
		for (;;) {
			if (!f || f(ptr->inst)){
				int dx = obj->x - ptr->inst->x;
				int dy = obj->y - ptr->inst->y;
				int dsqr = dx*dx+dy*dy;
				return ptr->inst;
			}
			ptr = ptr->next_in_grid;
			if (ptr == NULL)
				return NULL;
		}
	}*/
}
