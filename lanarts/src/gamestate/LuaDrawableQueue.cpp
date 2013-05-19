/*
 * LuaDrawableQueue.cpp:
 *  Efficiently stores lua drawables in descending order of depth.
 *  A lua drawable is either a closure, or an object with a draw method.
 *  The position at which to draw is passed as an argument.
 *
 *  This is cleared at the end of every draw phase.
 */

#include "lanarts_defines.h"

#include "gamestate/LuaDrawableQueue.h"
#include "util/lua_array_util.h"

void LuaDrawableQueue::add(const LuaField& lua_drawable, int depth,
		const PosF& position) {
	lua_State* L = lua_drawable.luastate();
	if (_drawable_array.empty()) {
		init(L);
	}

	/* Make the unlinked entry */
	luaarray_append(_drawable_array, lua_drawable);

	_LuaDrawableQueueEntry entry;
	entry.lua_drawable_index = _drawable_array.objlen(); //Note: lua indices start at 1
	entry.next_entry_index = -1;
	entry.position = position;

	_entry_pool.push_back(entry);

	/* Link the entry */
	link_entry(_entry_pool.size() - 1, depth);
}

void LuaDrawableQueue::clear() {
	luaarray_clear(_drawable_array);
}

void LuaDrawableQueue::link_entry(int entry_index, int depth) {
	_LuaDrawableList& list = _depth_to_list[depth];
	if (list.first_entry_index == -1) {
		list.first_entry_index = entry_index;
	} else {
		_entry_pool[list.last_entry_index].next_entry_index = entry_index;
	}
	list.last_entry_index = entry_index;
}

LuaDrawableQueue::Iterator LuaDrawableQueue::get_iterator() const {
	return Iterator(*this);
}

void LuaDrawableQueue::init(lua_State* L) {
	_drawable_array.init(L);
	_drawable_array.newtable();

	_cached_position.init(L);
	_cached_position.newtable();
}

void LuaDrawableQueue::Iterator::draw_current() const {
	lua_State* L = _queue._drawable_array.luastate();
	_queue._drawable_array.push();
	lua_rawgeti(L, -1, _current_entry->lua_drawable_index);
	if (lua_isfunction(L, -1)) {
		lua_getfield(L, -1, "draw");
		lua_pushvalue(L, -2);
		lua_call(L, 1, 0);
	} else {
		lua_call(L, 0, 0);
	}
}

LuaDrawableQueue::Iterator::Iterator(const LuaDrawableQueue& queue) :
				_queue(queue),
				_depth_map_iter(queue._depth_to_list.begin()) {
	if (_depth_map_iter != queue._depth_to_list.end()) {
		_current_entry = entry_at(_depth_map_iter->second.first_entry_index);
	} else {
		_current_entry = NULL;
	}
}

LuaField LuaDrawableQueue::Iterator::get_current_object() const {
	LANARTS_ASSERT(_current_entry != NULL && _depth_map_iter != _queue._depth_to_list.end());
	return _queue._drawable_array[_current_entry->lua_drawable_index];
}

void LuaDrawableQueue::Iterator::fetch_next() {
	LANARTS_ASSERT(_current_entry != NULL && _depth_map_iter != _queue._depth_to_list.end());

	_current_entry = entry_at(_current_entry->next_entry_index);
	if (_current_entry == NULL) {
		++_depth_map_iter;
		if (!is_done()) {
			_current_entry = entry_at(_depth_map_iter->second.first_entry_index);
		}
	}
}

bool LuaDrawableQueue::Iterator::is_done() const {
	return (_depth_map_iter == _queue._depth_to_list.end());
}

int LuaDrawableQueue::Iterator::get_current_depth() const {
	return _depth_map_iter->first;
}

const _LuaDrawableQueueEntry* LuaDrawableQueue::Iterator::entry_at(
		int idx) const {
	if (idx == -1) {
		return NULL;
	}
	return &_queue._entry_pool[idx];
}
